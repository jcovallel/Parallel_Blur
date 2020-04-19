#include "FreeImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

struct Blur_Params {
   FIBITMAP *img;
   FIBITMAP *imgAux;
   int kernel;
   unsigned ini;
   unsigned width;
};

/** FreeImage error handler @param fif Format / Plugin responsible for the error  @param message Error message */
void FreeImageErrorHandler( FREE_IMAGE_FORMAT fif, const char *message ){
   printf("\n*** ");
   if( fif != FIF_UNKNOWN ) printf( "%s Format\n", FreeImage_GetFormatFromFIF( fif ) );
   printf( "%s", message );
   printf( " ***\n" );
}

void *BlurFunc2( void *arg ){

   // Inicialización de variables
   struct Blur_Params *params = (struct Blur_Params *)arg;
   FIBITMAP *imagen = params -> img;
   int kernel = params -> kernel;
   unsigned width_ini = params -> ini;
   unsigned width  = params -> width;
   unsigned total_width = FreeImage_GetWidth( imagen );
   unsigned height = FreeImage_GetHeight( imagen );
   unsigned pitch  = FreeImage_GetPitch( imagen );
   BYTE *pixelAuxDer, *pixelAuxIz, *pixel, *bits, *recover = (BYTE *)FreeImage_GetBits( imagen );
   //se mueven los apuntadores a la columna inicial del hilo
   bits = recover = recover + ( 3 * width_ini );
   int radio = ( kernel - 1 ) / 2;
   int kernelLimit;
   // se inicializan los apuntadores a la imagen clonada para el kernel
   FIBITMAP *imagenAux = params -> imgAux;
   BYTE *bitsAux = (BYTE *)FreeImage_GetBits( imagenAux );
   bitsAux += ( 3 * width_ini );
   int sumRed, sumGreen, sumBlue, kernelCount, auxCount;

   /** Haremos primero un barrido horizontal y luego uno vertical de modo que
    * agilicemos la convolución del kernel con la imagen (si se hacen al Tiempo
    * es más lento O(K*K*h*w) vs O(2K*h*w))
    */

   //barrido horizontal
   for( int y = 0; y < height; y++ ){
      pixel = bits;
      kernelCount = 0;
      sumRed = sumGreen = sumBlue = 0;

      /** Se calcula el blur para el primer pixel (borde izquierdo)
       * El kernel de maneja sobre la imagen clonada y haciendo uso de dos apuntadores
       * uno izquierdo y uno derecho, que delimitan el inicio y el fin del vector kernel
       * (no matriz kernel ya que lo hacemos en dos barridos). El kernel se va moviento
       * sobre la imagen junto con el pixel que se está calculando.
       */
      pixelAuxDer = pixelAuxIz = bitsAux;
      kernelLimit = width_ini - radio - 1;

      // movemos el apuntador izquierdo del kernel
      auxCount = width_ini;
      while( auxCount >= 0 && auxCount > kernelLimit ){
         sumRed += pixelAuxIz[FI_RGBA_RED];
         sumGreen += pixelAuxIz[FI_RGBA_GREEN];
         sumBlue += pixelAuxIz[FI_RGBA_BLUE];
         pixelAuxIz -= 3;
         kernelCount++;
         auxCount--;
      }
      pixelAuxDer += 3;
      auxCount = width_ini + 1;
      kernelLimit = width_ini + radio + 1;
      // movemos el apuntador derecho del kernel
      while( auxCount < total_width && auxCount < kernelLimit ){
         sumRed += pixelAuxDer[FI_RGBA_RED];
         sumGreen += pixelAuxDer[FI_RGBA_GREEN];
         sumBlue += pixelAuxDer[FI_RGBA_BLUE];
         pixelAuxDer += 3;
         kernelCount++;
         auxCount++;
      }

      pixel[FI_RGBA_RED]=sumRed/kernelCount;
      pixel[FI_RGBA_GREEN]=sumGreen/kernelCount;
      pixel[FI_RGBA_BLUE]=sumBlue/kernelCount;
      pixel += 3;
      pixelAuxDer -= 3;
      pixelAuxIz += 3;
      //se calcula el blur para el resto de pixeles en la fila
      for( int x = width_ini + 1; x < width; x++ ){
         if( x - radio > 0 ){
            sumRed -= pixelAuxIz[FI_RGBA_RED];
            sumGreen -= pixelAuxIz[FI_RGBA_GREEN];
            sumBlue -= pixelAuxIz[FI_RGBA_BLUE];
            pixelAuxIz += 3;
            kernelCount--;
         }
         if( x + radio < total_width ){
            pixelAuxDer += 3;
            sumRed += pixelAuxDer[FI_RGBA_RED];
            sumGreen += pixelAuxDer[FI_RGBA_GREEN];
            sumBlue += pixelAuxDer[FI_RGBA_BLUE];
            kernelCount++;
         }
         pixel[FI_RGBA_RED] = sumRed / kernelCount;
         pixel[FI_RGBA_GREEN] = sumGreen / kernelCount;
         pixel[FI_RGBA_BLUE] = sumBlue / kernelCount;
         pixel += 3;
      }
      // siguiente línea
      bits += pitch;
      bitsAux += pitch;
   }
   // se inicializan apuntadores para iniciar el barrido vertical
   imagenAux = FreeImage_Clone( imagen );
   bitsAux = (BYTE *)FreeImage_GetBits( imagenAux );
   bitsAux += ( 3 * width_ini );
   bits = recover;
   //barrido vertical
   for( int x = width_ini; x < width; x++ ){
      pixel = bits;
      kernelCount = 0;
      sumRed = sumGreen = sumBlue = 0;

      //Se calcula el blur para el primer pixel (borde superior)
      pixelAuxDer = pixelAuxIz = bitsAux;
      kernelLimit = radio - 1;
      auxCount = 0; //inicializar en height_ini si se balancea carga en el eje y
      // se mueve el apuntador izquierdo del kernel
      while( auxCount >= 0 && auxCount > kernelLimit ){
         sumRed += pixelAuxIz[FI_RGBA_RED];
         sumGreen += pixelAuxIz[FI_RGBA_GREEN];
         sumBlue += pixelAuxIz[FI_RGBA_BLUE];
         pixelAuxIz -= pitch;
         kernelCount++;
         auxCount--;
      }
      pixelAuxDer += pitch;
      auxCount = 1; //inicializar en height_ini+1 si se balancea carga en el eje y
      kernelLimit = radio + 1;
      // se mueve el apuntador derecho del kernel
      while( auxCount < height && auxCount < kernelLimit ){
         sumRed += pixelAuxDer[FI_RGBA_RED];
         sumGreen += pixelAuxDer[FI_RGBA_GREEN];
         sumBlue += pixelAuxDer[FI_RGBA_BLUE];
         pixelAuxDer += pitch;
         kernelCount++;
         auxCount++;
      }

      pixel[FI_RGBA_RED]=sumRed/kernelCount;
      pixel[FI_RGBA_GREEN]=sumGreen/kernelCount;
      pixel[FI_RGBA_BLUE]=sumBlue/kernelCount;
      pixel += pitch;
      pixelAuxDer -= pitch;
      pixelAuxIz += pitch;
      //se calcula el blur para el resto de pixeles en la columna
      for( int y = 1; y < height; y++ ){
         if( y - radio > 0 ){
            sumRed -= pixelAuxIz[FI_RGBA_RED];
            sumGreen -= pixelAuxIz[FI_RGBA_GREEN];
            sumBlue -= pixelAuxIz[FI_RGBA_BLUE];
            pixelAuxIz += pitch;
            kernelCount--;
         }
         if( y + radio < height ){
            pixelAuxDer += pitch;
            sumRed += pixelAuxDer[FI_RGBA_RED];
            sumGreen += pixelAuxDer[FI_RGBA_GREEN];
            sumBlue += pixelAuxDer[FI_RGBA_BLUE];
            kernelCount++;
         }
         pixel[FI_RGBA_RED]=sumRed/kernelCount;
         pixel[FI_RGBA_GREEN]=sumGreen/kernelCount;
         pixel[FI_RGBA_BLUE]=sumBlue/kernelCount;
         pixel += pitch;
      }
      // siguiente linea
      bits += 3;
      bitsAux += 3;
   }
   FreeImage_Unload( imagenAux );
}


int main( int argc, char *argv[] ){

   // verificamos si se recibieron todos los argumentos
   if( argc != 5 ){
      printf( "Please provide all arguments: blur-effect sourceImage outputImage kernelSize coreNum");
      exit( 1 );
   }

   // inicialización de la librería FreeImage
   FreeImage_Initialise( FALSE );

   FREE_IMAGE_FORMAT formato = FreeImage_GetFileType( argv[1], 0 );
   if( formato  == FIF_UNKNOWN ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   // Se carga la imagen a un mapa de bits
   FIBITMAP* imagen = FreeImage_Load( formato, argv[1], 0 );
   if( !imagen ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   unsigned total_width  = FreeImage_GetWidth(imagen); // ancho de la imagen
   unsigned total_height = FreeImage_GetHeight(imagen); // alto de la imagen
   int hilos = atoi(argv[4]);
   int *retval;

   int block_width = total_width / hilos; // balanceo de carga blockwise (se divide la img por columnas)

   FIBITMAP *imagenAux = FreeImage_Clone( imagen ); // se clona la imagen a otro bitmap para sacar de ella los valores del kernel
   struct Blur_Params *params = malloc( sizeof( struct Blur_Params ) * hilos );

   // se lanzan los hilos con posición inicial y final en el eje x para dividir la imagen
   for( int i = 0; i < hilos - 1; i++ ){
      params[i].img = imagen;
      params[i].imgAux = imagenAux;
      params[i].kernel = atoi(argv[3]);
      params[i].ini = block_width * i;
      params[i].width = block_width * (i+1);
      //pthread_create( &thread[i], NULL, (void *)BlurFunc2, (void *)&params[i] );
   }

   // se lanza el último hilo que incluye el resto de columnas (la división no da entera siempre)
   params[hilos - 1].img = imagen;
   params[hilos - 1].imgAux = imagenAux;
   params[hilos - 1].kernel = atoi(argv[3]);
   params[hilos - 1].ini = block_width * ( hilos - 1 );
   params[hilos - 1].width = total_width;
   //pthread_create( &thread[hilos - 1], NULL, (void *)BlurFunc2, (void *)&params[hilos - 1] );

#pragma omp parallel num_threads(hilos) //inicio de region paralela
{
   int ID = omp_get_thread_num();
   BlurFunc2( (void *)&params[ID] );
}

   // se guardan los cambios en una nueva imagen
   if ( FreeImage_Save( FIF_BMP, imagen, argv[2], 0 ) ) {
   }

   // liberar memoria y desinicializar la librería
   free( params );
   FreeImage_Unload( imagen );
   FreeImage_Unload( imagenAux );
   FreeImage_DeInitialise( );
}
