// gcc -Wall -I./ -o blur blur.c FreeImage.lib

#include "FreeImage.h"
#include <stdio.h>
#include <stdlib.h>


/** FreeImage error handler @param fif Format / Plugin responsible for the error  @param message Error message */
void FreeImageErrorHandler( FREE_IMAGE_FORMAT fif, const char *message ){
   printf("\n*** ");
   if( fif != FIF_UNKNOWN ){
      printf( "%s Format\n", FreeImage_GetFormatFromFIF( fif ) );
   }
   printf( message );
   printf( " ***\n" );
}

void BlurFunc( BYTE *bits, BYTE *bits2, unsigned height, unsigned width, unsigned pitch, int kernel ){
   int radio = ( kernel - 1 ) / 2;
   int sumRed, sumGreen, sumBlue;
   BYTE *firstln = (BYTE*)bits;
   BYTE *firstln2 = (BYTE*)bits2;

   for( int y = 0; y < height; y++ ){
      BYTE *pixel = (BYTE*)bits;
      BYTE *pixel2 = (BYTE*)bits2;
      for( int x = 0; x < width; x++ ){

         sumRed = sumGreen = sumBlue = 0;

         int h_init = x - radio;
         if ( h_init < 0 ) h_init = 0;
         int h_fin = x + radio;
         if ( h_fin >= width ) h_fin = width-1;

         int v_init = y - radio;
         if ( v_init < 0 ) v_init = 0;
         int v_fin = y + radio;
         if ( v_fin >= height ) v_fin = height-1;
         BYTE *vertical = (BYTE*)firstln;
         vertical += (pitch*v_init);
         BYTE *horizontal = (BYTE*)vertical;
         horizontal += (3*h_init);
         int count=0;

         for(int ini_v = v_init; ini_v <= v_fin; ini_v++){
            horizontal = vertical;
            horizontal += (3*h_init);
            for(int ini_h = h_init; ini_h <= h_fin; ini_h++){
               sumRed += horizontal[FI_RGBA_RED];
               sumGreen += horizontal[FI_RGBA_GREEN];
               sumBlue += horizontal[FI_RGBA_BLUE];
               horizontal += 3;
               count++;
            }
            vertical += pitch;
         }
         pixel2[FI_RGBA_RED]=(int)(sumRed/count);
         pixel2[FI_RGBA_GREEN]=(int)(sumGreen/count);
         pixel2[FI_RGBA_BLUE]=(int)(sumBlue/count);
         pixel += 3;
         pixel2 += 3;
      }
      // next line
      bits += pitch;
      bits2 += pitch;
   }
}

void BlurFunc2( BYTE *bits, BYTE *bits2, unsigned height, unsigned width, unsigned pitch, int kernel ){
   int radio = ( kernel - 1 ) / 2;
   int sumRed, sumGreen, sumBlue;
   //barrido horizontal
   for( int y = 0; y < height; y++ ){
      BYTE *pixel = (BYTE*)bits;
      BYTE *pixel2 = (BYTE*)bits2;
      for( int x = 0; x < width; x++ ){

         sumRed = sumGreen = sumBlue = 0;
         int count = 0;
         int h_init = x - radio;
         if ( h_init < 0 ) h_init = 0;
         int h_fin = x + radio;
         if ( h_fin >= width ) h_fin = width-1;
         BYTE *pixelAux = pixel-((x-h_init)*3);
         for(int ini_h = h_init; ini_h <= h_fin; ini_h++){

            sumRed += pixelAux[FI_RGBA_RED];
            sumGreen += pixelAux[FI_RGBA_GREEN];
            sumBlue += pixelAux[FI_RGBA_BLUE];
            pixelAux += 3;
            count++;
         }

         pixel2[FI_RGBA_RED]=(sumRed/count);
         pixel2[FI_RGBA_GREEN]=(sumGreen/count);
         pixel2[FI_RGBA_BLUE]=(sumBlue/count);
         pixel += 3;
         pixel2 += 3;
      }
      // next line
      bits += pitch;
      bits2 += pitch;
   }
//barrido vertical
   BYTE *bits3 = (BYTE *)malloc(sizeof(bits2));
   *bits3 = *bits2;
   for( int x = 0; x < width; x++ ){
      BYTE *pixel = (BYTE*)bits3;
      BYTE *pixel2 = (BYTE*)bits2;
      for( int y = 0; y < height; y++ ){

         sumRed = sumGreen = sumBlue = 0;
         int count = 0;
         int v_init = y - radio;
         if ( v_init < 0 ) v_init = 0;
         int v_fin = y + radio;
         if ( v_fin >= height ) v_fin = height-1;
         BYTE *pixelAux = pixel-((x-v_init)*pitch);
         for(int ini_v = v_init; ini_v <= v_fin; ini_v++){

            sumRed += pixelAux[FI_RGBA_RED];
            sumGreen += pixelAux[FI_RGBA_GREEN];
            sumBlue += pixelAux[FI_RGBA_BLUE];
            pixelAux += pitch;
            count++;
         }

         pixel2[FI_RGBA_RED]=(sumRed/count);
         pixel2[FI_RGBA_GREEN]=(sumGreen/count);
         pixel2[FI_RGBA_BLUE]=(sumBlue/count);
         pixel += pitch;
         pixel2 += pitch;
      }
      // next line
      bits3 += 3;
      bits2 += 3;
   }
   free(bits3);
}

int main( int argc, char *argv[] )
{
   if( argc != 5 ){
      printf( "Please provide all arguments: blur-effect sourceImage outputImage kernelSize coreNum");
      exit( 1 );
   }
   FreeImage_Initialise( FALSE );
   //printf( "%s", argv[1] );
   FREE_IMAGE_FORMAT formato = FreeImage_GetFileType( argv[1], 0 );
   if( formato  == FIF_UNKNOWN ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }
   //printf( "%s", FreeImage_GetFormatFromFIF( formato ) );

   FIBITMAP* imagen = FreeImage_Load( formato, argv[1], 0 );
   FIBITMAP* imagen2 = FreeImage_Load( formato, argv[1], 0 );
   if( !imagen || !imagen2){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }



   unsigned width  = FreeImage_GetWidth(imagen);
   unsigned height = FreeImage_GetHeight(imagen);
   unsigned pitch  = FreeImage_GetPitch(imagen);

   FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(imagen);

   //printf( "%s",image_type == FIT_BITMAP ? "true" : "false" );

// test pixel access avoiding scanline calculations to speed-up the image processing
/* different image type
   if(image_type == FIT_RGBF){
      BYTE *bits = (BYTE*)FreeImage_GetBits(imagen);
      for(y = 0; y < height; y++){
         FIRGBF *pixel = (FIRGBF*)bits;
         for(x = 0; x < width; x++){
            pixel[x].red = 128;
            pixel[x].green = 128;
            pixel[x].blue = 128;
         }
         // next line
         bits += pitch;
      }
   }
   */
   if( ( image_type == FIT_BITMAP ) && ( FreeImage_GetBPP( imagen ) == 24 ) ){
      BYTE *bits = (BYTE*)FreeImage_GetBits( imagen );
      BYTE *bits2 = (BYTE*)FreeImage_GetBits( imagen2 );

      BlurFunc(bits, bits2, height, width, pitch, atoi(argv[3]));
   }

   if (FreeImage_Save(FIF_BMP, imagen2, argv[2], 0)) {     // bitmap successfully saved!
   }

   FreeImage_Unload( imagen );

   /*
   FIBITMAP* temp = FreeImage_ConvertTo32Bits( imagen );
   if( !imagen ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   FreeImage_Unload( imagen );
   imagen = temp;
   */
   FreeImage_DeInitialise( );
}
