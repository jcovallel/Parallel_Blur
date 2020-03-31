// gcc -Wall -I./ -o blur blur.c FreeImage.lib

#include "FreeImage.h"
#include <stdio.h>
#include <stdlib.h>

/** Queue implementation */
struct nodo {
    int info;
    struct nodo *sig;
};

struct cola {
   int numElementosEnLista;
   int sumatoria;
   struct nodo *raiz;
   struct nodo *fondo;
};

int vacia( struct cola *miCola ){
   return miCola->numElementosEnLista==0 ? 1 : 0;
}

void insertar(int x, struct cola *miCola){
   struct nodo *nuevo;
   nuevo=(struct nodo *)malloc(sizeof(struct nodo));
   nuevo->info=x;
   nuevo->sig=NULL;
   if (vacia( miCola )){
       miCola->raiz = nuevo;
       miCola->fondo = nuevo;
   }
   else {
       miCola->fondo->sig = nuevo;
       miCola->fondo = nuevo;
   }
   miCola->numElementosEnLista++;
   miCola->sumatoria = miCola->sumatoria + x;
}

int extraer( struct cola *miCola ){
   if (!vacia( miCola )){
      int informacion = miCola->raiz->info;
      struct nodo *bor = miCola->raiz;
      if (miCola->raiz == miCola->fondo){
         miCola->raiz = NULL;
         miCola->fondo = NULL;
      }
      else {
         miCola->raiz = miCola->raiz->sig;
      }
      free(bor);
      miCola->numElementosEnLista--;
      miCola->sumatoria = miCola->sumatoria - informacion;
      return informacion;
    }
    else return -1;
}

void liberar( struct cola *miCola ){
    struct nodo *reco = miCola->raiz;
    struct nodo *bor;
    while (reco != NULL){
        bor = reco;
        reco = reco->sig;
        free(bor);
    }
}



/** FreeImage error handler @param fif Format / Plugin responsible for the error  @param message Error message */
void FreeImageErrorHandler( FREE_IMAGE_FORMAT fif, const char *message ){
   printf("\n*** ");
   if( fif != FIF_UNKNOWN ) printf( "%s Format\n", FreeImage_GetFormatFromFIF( fif ) );
   printf( message );
   printf( " ***\n" );
}


void BlurFunc( FIBITMAP* imagen, int kernel ){
   unsigned width  = FreeImage_GetWidth( imagen );
   unsigned height = FreeImage_GetHeight( imagen );
   unsigned pitch  = FreeImage_GetPitch( imagen );
   BYTE *pixelAux, *pixel, *bits, *recover=(BYTE*)FreeImage_GetBits( imagen );
   bits = recover;
   int radio = ( kernel - 1 ) / 2;
   int h_fin, v_fin;
   struct cola *colaRed = (struct cola *)malloc(sizeof(struct cola));
   struct cola *colaGreen = (struct cola *)malloc(sizeof(struct cola));
   struct cola *colaBlue = (struct cola *)malloc(sizeof(struct cola));
   colaRed->raiz = NULL;
   colaBlue->raiz = NULL;
   colaGreen->raiz = NULL;
   colaRed->fondo = NULL;
   colaBlue->fondo = NULL;
   colaGreen->fondo = NULL;
   //barrido horizontal
   for( int y = 0; y < height; y++ ){
      pixel = bits;
      colaRed->numElementosEnLista = 0;
      colaBlue->numElementosEnLista = 0;
      colaGreen->numElementosEnLista = 0;
      colaRed->sumatoria = 0;
      colaBlue->sumatoria = 0;
      colaGreen->sumatoria = 0;

      //Se calcula el blur para el primer pixel (borde izquierdo)
      pixelAux = pixel;
      h_fin = radio + 1;
      for( int s = 0; s < h_fin; s++ ){
         insertar( pixelAux[FI_RGBA_RED], colaRed );
         insertar( pixelAux[FI_RGBA_GREEN], colaGreen );
         insertar( pixelAux[FI_RGBA_BLUE], colaBlue );
         pixelAux += 3;
      }
      pixel[FI_RGBA_RED]=colaRed->sumatoria/colaRed->numElementosEnLista;
      pixel[FI_RGBA_GREEN]=colaGreen->sumatoria/colaGreen->numElementosEnLista;
      pixel[FI_RGBA_BLUE]=colaBlue->sumatoria/colaBlue->numElementosEnLista;
      pixel += 3;
      pixelAux -= 3;
      //se calcula el blur para el resto de pixeles en la fila
      for( int x = 1; x < width; x++ ){
         if( x - radio > 0 ){
            extraer( colaRed );
            extraer( colaGreen );
            extraer( colaBlue );
         }
         if( x + radio < width){
            pixelAux += 3;
            insertar( pixelAux[FI_RGBA_RED], colaRed );
            insertar( pixelAux[FI_RGBA_GREEN], colaGreen );
            insertar( pixelAux[FI_RGBA_BLUE], colaBlue );
         }
         pixel[FI_RGBA_RED]=colaRed->sumatoria/colaRed->numElementosEnLista;
         pixel[FI_RGBA_GREEN]=colaGreen->sumatoria/colaGreen->numElementosEnLista;
         pixel[FI_RGBA_BLUE]=colaBlue->sumatoria/colaBlue->numElementosEnLista;
         pixel += 3;
      }
      // next line
      bits += pitch;
   }

   liberar( colaRed );
   liberar( colaGreen );
   liberar( colaBlue );
   colaRed->raiz = NULL;
   colaBlue->raiz = NULL;
   colaGreen->raiz = NULL;
   colaRed->fondo = NULL;
   colaBlue->fondo = NULL;
   colaGreen->fondo = NULL;
   //barrido vertical
   bits = recover;
   for( int x = 0; x < width; x++ ){
      pixel = bits;
      colaRed->numElementosEnLista = 0;
      colaBlue->numElementosEnLista = 0;
      colaGreen->numElementosEnLista = 0;
      colaRed->sumatoria = 0;
      colaBlue->sumatoria = 0;
      colaGreen->sumatoria = 0;

      //Se calcula el blur para el primer pixel (borde superior)
      pixelAux = pixel;
      v_fin = radio + 1;
      for( int s = 0; s < v_fin; s++ ){
         insertar( pixelAux[FI_RGBA_RED], colaRed );
         insertar( pixelAux[FI_RGBA_GREEN], colaGreen );
         insertar( pixelAux[FI_RGBA_BLUE], colaBlue );
         pixelAux += pitch;
      }
      pixel[FI_RGBA_RED]=colaRed->sumatoria/colaRed->numElementosEnLista;
      pixel[FI_RGBA_GREEN]=colaGreen->sumatoria/colaGreen->numElementosEnLista;
      pixel[FI_RGBA_BLUE]=colaBlue->sumatoria/colaBlue->numElementosEnLista;
      pixel += pitch;
      pixelAux -= pitch;
      //se calcula el blur para el resto de pixeles en la fila
      for( int y = 1; y < height; y++ ){
         if( y - radio > 0 ){
            extraer( colaRed );
            extraer( colaGreen );
            extraer( colaBlue );
         }
         if( y + radio < height ){
            pixelAux += pitch;
            insertar( pixelAux[FI_RGBA_RED], colaRed );
            insertar( pixelAux[FI_RGBA_GREEN], colaGreen );
            insertar( pixelAux[FI_RGBA_BLUE], colaBlue );
         }
         pixel[FI_RGBA_RED]=colaRed->sumatoria/colaRed->numElementosEnLista;
         pixel[FI_RGBA_GREEN]=colaGreen->sumatoria/colaGreen->numElementosEnLista;
         pixel[FI_RGBA_BLUE]=colaBlue->sumatoria/colaBlue->numElementosEnLista;
         pixel += pitch;
      }
      // next line
      bits += 3;
   }
   liberar( colaRed );
   liberar( colaGreen );
   liberar( colaBlue );
   free( colaRed );
   free( colaGreen );
   free( colaBlue );
}

void BlurFunc2( FIBITMAP* imagen, int kernel ){
   unsigned width  = FreeImage_GetWidth( imagen );
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
      h_fin = radio + 1;
      for( int s = 0; s < h_fin; s++ ){
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
      for( int x = 1; x < width; x++ ){
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
   for( int x = 0; x < width; x++ ){
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
}


void BlurFunc3( FIBITMAP* imagen, int kernel ){
   unsigned width  = FreeImage_GetWidth( imagen );
   unsigned height = FreeImage_GetHeight( imagen );
   unsigned pitch  = FreeImage_GetPitch( imagen );
   BYTE *bits, *recover=(BYTE*)FreeImage_GetBits( imagen );
   bits = recover;
   FIBITMAP *imagen2 = FreeImage_Clone( imagen );
   BYTE *bits2 = (BYTE*)FreeImage_GetBits( imagen2 );
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
         BYTE *pixelAux = pixel2-((x-h_init)*3);
         for(int ini_h = h_init; ini_h <= h_fin; ini_h++){

            sumRed += pixelAux[FI_RGBA_RED];
            sumGreen += pixelAux[FI_RGBA_GREEN];
            sumBlue += pixelAux[FI_RGBA_BLUE];
            pixelAux += 3;
            count++;
         }

         pixel[FI_RGBA_RED]=sumRed/count;
         pixel[FI_RGBA_GREEN]=sumGreen/count;
         pixel[FI_RGBA_BLUE]=sumBlue/count;
         pixel += 3;
         pixel2 += 3;
      }
      // next line
      bits += pitch;
      bits2 += pitch;
   }
   FreeImage_Unload( imagen2 );
   //barrido vertical
   FIBITMAP *imagen3 = FreeImage_Clone(imagen);
   BYTE *bits3 = (BYTE*)FreeImage_GetBits( imagen3 );
   bits = recover;
   for( int x = 0; x < width; x++ ){
      BYTE *pixel = (BYTE*)bits3;
      BYTE *pixel2 = (BYTE*)bits;
      for( int y = 0; y < height; y++ ){

         sumRed = sumGreen = sumBlue = 0;
         int count = 0;
         int v_init = y - radio;
         if ( v_init < 0 ) v_init = 0;
         int v_fin = y + radio;
         if ( v_fin >= height ) v_fin = height-1;
         BYTE *pixelAux = pixel-((y-v_init)*pitch);
         for(int ini_v = v_init; ini_v <= v_fin; ini_v++){

            sumRed += pixelAux[FI_RGBA_RED];
            sumGreen += pixelAux[FI_RGBA_GREEN];
            sumBlue += pixelAux[FI_RGBA_BLUE];
            pixelAux += pitch;
            count++;
         }

         pixel2[FI_RGBA_RED]=sumRed/count;
         pixel2[FI_RGBA_GREEN]=sumGreen/count;
         pixel2[FI_RGBA_BLUE]=sumBlue/count;
         pixel += pitch;
         pixel2 += pitch;
      }
      // next line
      bits3 += 3;
      bits += 3;
   }
   FreeImage_Unload( imagen3 );
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

   BlurFunc2(imagen, atoi(argv[3]));
   if (FreeImage_Save(FIF_BMP, imagen, argv[2], 0)) {     // bitmap successfully saved!
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
