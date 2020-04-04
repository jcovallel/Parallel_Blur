#include "FreeImage.h"
#include <stdio.h>
#include <stdlib.h>

/** FreeImage error handler @param fif Format / Plugin responsible for the error  @param message Error message */
void FreeImageErrorHandler( FREE_IMAGE_FORMAT fif, const char *message ){
   printf("\n*** ");
   if( fif != FIF_UNKNOWN ) printf( "%s Format\n", FreeImage_GetFormatFromFIF( fif ) );
   printf( "%s", message );
   printf( " ***\n" );
}

int main( int argc, char *argv[] ){
   if( argc != 4 ){
      printf( "Please provide all arguments: blur-effect sourceImage outputImage kernelSize coreNum");
      exit( 1 );
   }
   FreeImage_Initialise( FALSE );

   FREE_IMAGE_FORMAT formato = FreeImage_GetFileType( argv[1], 0 );
   if( formato  == FIF_UNKNOWN ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   FREE_IMAGE_FORMAT formato2 = FreeImage_GetFileType( argv[2], 0 );
   if( formato  == FIF_UNKNOWN ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   FIBITMAP* imagen = FreeImage_Load( formato, argv[1], 0 );
   if( !imagen ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   FIBITMAP* imagen2 = FreeImage_Load( formato2, argv[2], 0 );
   if( !imagen2 ){
      FreeImage_SetOutputMessage( FreeImageErrorHandler );
   }

   FIBITMAP* imagenAux = FreeImage_Clone( imagen2 );

   unsigned total_width  = FreeImage_GetWidth(imagen);
   unsigned total_height = FreeImage_GetHeight(imagen);
   unsigned pitch  = FreeImage_GetPitch( imagen );

   if(total_width!=FreeImage_GetWidth(imagen2)){
      printf("paila mk no coincide el ancho de las imagenes\n");
   }
   if(total_height!=FreeImage_GetHeight(imagen2)){
      printf("paila mk no coincide el alto de las imagenes\n");
   }

   BYTE *bits, *recover = (BYTE*)FreeImage_GetBits( imagen );
   BYTE *bits2, *recover2 = (BYTE*)FreeImage_GetBits( imagen2 );
   BYTE *bitsAux, *recover3 = (BYTE *)FreeImage_GetBits( imagenAux );

   int error = 0;
   FILE * fp;
   /* open the file for writing*/
   fp = fopen ("comparacion.txt","w");

   for(int i = 0; i<total_height; i++){
      bits = recover;
      bits2 = recover2;
      bitsAux = recover3;
      for(int k =0; k<total_width; k++){
         if(bits[FI_RGBA_RED]!=bits2[FI_RGBA_RED]){
            bitsAux[FI_RGBA_RED]=191;
            fprintf (fp, "Posicion x=%d, y=%d | r original=%d | r opti=%d\n",k,i,bits[FI_RGBA_RED],bits2[FI_RGBA_RED]);
            error=1;
         }

         if(bits[FI_RGBA_GREEN]!=bits2[FI_RGBA_GREEN]){
            bitsAux[FI_RGBA_GREEN]=255;
            fprintf (fp, "Posicion x=%d, y=%d | g original=%d | g opti=%d\n",k,i,bits[FI_RGBA_GREEN],bits2[FI_RGBA_GREEN]);
            error=1;
         }

         if(bits[FI_RGBA_BLUE]!=bits2[FI_RGBA_BLUE]){
            bitsAux[FI_RGBA_BLUE]=0;
            fprintf (fp, "Posicion x=%d, y=%d | b original=%d | b opti=%d\n",k,i,bits[FI_RGBA_BLUE],bits2[FI_RGBA_BLUE]);
            error=1;
         }

         bits+=3;
         bits2+=3;
         bitsAux+=3;
      }
      recover+=pitch;
      recover2+=pitch;
      recover3+=pitch;
   }

   if(error){
      printf("Existe alguna diferencia \n" );
   }

   if (FreeImage_Save(FIF_BMP, imagenAux, argv[3], 0)) {     // bitmap successfully saved!
   }

   /* close the file*/
   fclose (fp);
   FreeImage_Unload( imagen );
   FreeImage_Unload( imagen2 );
   FreeImage_DeInitialise( );
}
