#include "FreeImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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

   struct Blur_Params *params = (struct Blur_Params *)arg;
   FIBITMAP *imagen = params -> img;
   int kernel = params -> kernel;
   unsigned width_ini = params -> ini;
   unsigned width  = params -> width;
   unsigned total_width = FreeImage_GetWidth( imagen );
   unsigned height = FreeImage_GetHeight( imagen );
   unsigned pitch  = FreeImage_GetPitch( imagen );
   BYTE *pixelAuxDer, *pixelAuxIz, *pixel, *bits, *recover = (BYTE *)FreeImage_GetBits( imagen );
   bits = recover = recover + ( 3 * width_ini );
   int radio = ( kernel - 1 ) / 2;
   int kernelLimit;
   FIBITMAP *imagenAux = params -> imgAux;
   //FIBITMAP *imagenAux = FreeImage_Clone(imagen);
   BYTE *bitsAux = (BYTE *)FreeImage_GetBits( imagenAux );
   bitsAux += ( 3 * width_ini );
   int sumRed, sumGreen, sumBlue, kernelCount, auxCount;

   //barrido horizontal
   for( int y = 0; y < height; y++ ){
      pixel = bits;
      kernelCount = 0;
      sumRed = sumGreen = sumBlue = 0;

      //Se calcula el blur para el primer pixel (borde izquierdo)
      pixelAuxDer = pixelAuxIz = bitsAux;
      kernelLimit = width_ini - radio - 1;

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
      // next line
      bits += pitch;
      bitsAux += pitch;
   }
   //FreeImage_Unload( imagenAux );
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
      // next line
      bits += 3;
      bitsAux += 3;
   }
   FreeImage_Unload( imagenAux );
}


int main( int argc, char *argv[] ){
   if( argc != 5 ){
      printf( "Please provide all arguments: blur-effect sourceImage outputImage kernelSize coreNum");
      exit( 1 );
   }
   FreeImage_Initialise( FALSE );

   FREE_IMAGE_FORMAT formato = FreeImage_GetFileType( argv[1], 0 );
   if( formato  == FIF_UNKNOWN ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   FIBITMAP* imagen = FreeImage_Load( formato, argv[1], 0 );
   if( !imagen ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   unsigned total_width  = FreeImage_GetWidth(imagen);
   unsigned total_height = FreeImage_GetHeight(imagen);
   int hilos = atoi(argv[4]);
   int *retval;

   int block_width = total_width / hilos;
   pthread_t thread[hilos];
   FIBITMAP *imagenAux = FreeImage_Clone( imagen );
   struct Blur_Params *params = malloc( sizeof( struct Blur_Params ) * hilos );

   for( int i = 0; i < hilos - 1; i++ ){
      params[i].img = imagen;
      params[i].imgAux = imagenAux;
      params[i].kernel = atoi(argv[3]);
      params[i].ini = block_width * i;
      params[i].width = block_width * (i+1);
      pthread_create( &thread[i], NULL, (void *)BlurFunc2, (void *)&params[i] );
   }
   params[hilos - 1].img = imagen;
   params[hilos - 1].imgAux = imagenAux;
   params[hilos - 1].kernel = atoi(argv[3]);
   params[hilos - 1].ini = block_width * ( hilos - 1 );
   //params[hilos-1].width = ( block_width * hilos ) + total_width % hilos;
   params[hilos - 1].width = total_width;

   pthread_create( &thread[hilos - 1], NULL, (void *)BlurFunc2, (void *)&params[hilos - 1] );

   for( int i = 0; i < hilos; i++ ){
      pthread_join( thread[i], NULL );
   }



   if ( FreeImage_Save( FIF_BMP, imagen, argv[2], 0 ) ) {     // bitmap successfully saved!
   }

   free( params );
   FreeImage_Unload( imagen );
   FreeImage_Unload( imagenAux );
   FreeImage_DeInitialise( );
}
