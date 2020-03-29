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
   if( !imagen ){
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
      for( int y = 0; y < height; y++ ){
         BYTE *pixel = (BYTE*)bits;
         for( int x = 0; x < width; x++ ){
            pixel[FI_RGBA_RED] = 128;
            pixel[FI_RGBA_GREEN] = 128;
            pixel[FI_RGBA_BLUE] = 128;
            pixel += 3;
         }
         // next line
         bits += pitch;
      }
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
