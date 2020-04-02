#include "FreeImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct Blur_Params {
   FIBITMAP* img;
   int kernel;
   unsigned ini;
   unsigned width;
};

/** FreeImage error handler @param fif Format / Plugin responsible for the error  @param message Error message */
void FreeImageErrorHandler( FREE_IMAGE_FORMAT fif, const char *message ){
   printf("\n*** ");
   if( fif != FIF_UNKNOWN ) printf( "%s Format\n", FreeImage_GetFormatFromFIF( fif ) );
   printf( message );
   printf( " ***\n" );
}

void *BlurFunc2( void *arg ){

   struct Blur_Params *params = arg;
   FIBITMAP* imagen = params -> img;
   int kernel = params -> kernel;
   unsigned width_ini = params -> ini;
   unsigned width  = params -> width;

   printf("width_ini: %d, width: %d\n",width_ini,width );

   unsigned height = FreeImage_GetHeight( imagen );
   unsigned pitch  = FreeImage_GetPitch( imagen );
   BYTE *pixelAuxDer, *pixelAuxIz, *pixel, *bits, *recover=(BYTE*)FreeImage_GetBits( imagen );
   bits = recover;
   int radio = ( kernel - 1 ) / 2;
   int h_fin, v_fin;
   FIBITMAP* imagenAux = FreeImage_Clone( imagen );
   BYTE *bitsAux = (BYTE *)FreeImage_GetBits( imagenAux );
   int sumRed, sumGreen, sumBlue, kernelCount;

   //barrido horizontal
   for( int y = 0; y < height; y++ ){
      pixel = bits;
      kernelCount = 0;
      sumRed = sumGreen = sumBlue = 0;

      //Se calcula el blur para el primer pixel (borde izquierdo)
      pixelAuxDer = pixelAuxIz = bitsAux;
      h_fin = width_ini + radio + 1;
      for( int s = width_ini; s < h_fin; s++ ){
         sumRed += pixelAuxDer[FI_RGBA_RED];
         sumGreen += pixelAuxDer[FI_RGBA_GREEN];
         sumBlue += pixelAuxDer[FI_RGBA_BLUE];
         pixelAuxDer += 3;
         kernelCount++;
      }
      pixel[FI_RGBA_RED]=sumRed/kernelCount;
      pixel[FI_RGBA_GREEN]=sumGreen/kernelCount;
      pixel[FI_RGBA_BLUE]=sumBlue/kernelCount;
      pixel += 3;
      pixelAuxDer -= 3;
      //se calcula el blur para el resto de pixeles en la fila
      for( int x = width_ini + 1; x < width; x++ ){
         if( x - radio > 0 ){
            sumRed -= pixelAuxIz[FI_RGBA_RED];
            sumGreen -= pixelAuxIz[FI_RGBA_GREEN];
            sumBlue -= pixelAuxIz[FI_RGBA_BLUE];
            pixelAuxIz += 3;
            kernelCount--;
         }
         if( x + radio < width){
            pixelAuxDer += 3;
            sumRed += pixelAuxDer[FI_RGBA_RED];
            sumGreen += pixelAuxDer[FI_RGBA_GREEN];
            sumBlue += pixelAuxDer[FI_RGBA_BLUE];
            kernelCount++;
         }
         pixel[FI_RGBA_RED]=sumRed/kernelCount;
         pixel[FI_RGBA_GREEN]=sumGreen/kernelCount;
         pixel[FI_RGBA_BLUE]=sumBlue/kernelCount;
         pixel += 3;
      }
      // next line
      bits += pitch;
      bitsAux += pitch;
   }

   //barrido vertical
   FreeImage_Unload( imagenAux );
   imagenAux = FreeImage_Clone( imagen );
   bitsAux = (BYTE *)FreeImage_GetBits( imagenAux );
   bits = recover;
   for( int x = width_ini; x < width; x++ ){
      pixel = bits;
      kernelCount = 0;
      sumRed = sumGreen = sumBlue = 0;

      //Se calcula el blur para el primer pixel (borde izquierdo)
      pixelAuxDer = pixelAuxIz = bitsAux;
      v_fin = radio + 1;
      for( int s = 0; s < v_fin; s++ ){
         sumRed += pixelAuxDer[FI_RGBA_RED];
         sumGreen += pixelAuxDer[FI_RGBA_GREEN];
         sumBlue += pixelAuxDer[FI_RGBA_BLUE];
         pixelAuxDer += pitch;
         kernelCount++;
      }
      pixel[FI_RGBA_RED]=sumRed/kernelCount;
      pixel[FI_RGBA_GREEN]=sumGreen/kernelCount;
      pixel[FI_RGBA_BLUE]=sumBlue/kernelCount;
      pixel += pitch;
      pixelAuxDer -= pitch;
      //se calcula el blur para el resto de pixeles en la fila
      for( int y = 1; y < height; y++ ){
         if( y - radio > 0 ){
            sumRed -= pixelAuxIz[FI_RGBA_RED];
            sumGreen -= pixelAuxIz[FI_RGBA_GREEN];
            sumBlue -= pixelAuxIz[FI_RGBA_BLUE];
            pixelAuxIz += pitch;
            kernelCount--;
         }
         if( y + radio < height){
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
   return 0;
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
   printf("hilos: %d\n",hilos );
   int *retval;

   int block_width = total_width/hilos;
   pthread_t thread[hilos];

   struct Blur_Params *blur_params;
   blur_params = malloc(sizeof(*blur_params));
   blur_params -> img = imagen;
   blur_params -> kernel = atoi(argv[3]);

   for( int i = 0; i < hilos-1; i++ ){
      blur_params -> ini = block_width * i;
      blur_params -> width = block_width * (i+1);
      //printf("%u, %u\n", blur_params -> ini, blur_params -> width);
      pthread_create(&thread[i], NULL, (void *) BlurFunc2, blur_params);
   }
   blur_params -> ini = block_width * ( hilos - 1 );
   blur_params -> width = block_width * hilos;
   pthread_create(&thread[hilos - 1], NULL, (void *) BlurFunc2, blur_params);
   //printf("%u, %u\n", blur_params -> ini, blur_params -> width);

   for( int i = 0; i < hilos; i++ ){
      pthread_join(thread[i], NULL);
   }

   if (FreeImage_Save(FIF_BMP, imagen, argv[2], 0)) {     // bitmap successfully saved!
   }

   free(blur_params);
   FreeImage_Unload( imagen );
   FreeImage_DeInitialise( );
}
