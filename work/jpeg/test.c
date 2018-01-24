/*
 * example.c
 *
 * This file illustrates how to use the IJG code as a subroutine library
 * to read or write JPEG image files.  You should look at this code in
 * conjunction with the documentation file libjpeg.txt.
 *
 * This code will not do anything useful as-is, but it may be helpful as a
 * skeleton for constructing routines that call the JPEG library.  
 *
 * We present these routines in the same coding style used in the JPEG code
 * (ANSI function definitions, etc); but you are of course free to code your
 * routines in a different style if you prefer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * Include file for users of JPEG library.
 * You will need to have included system headers that define at least
 * the typedefs FILE and size_t before you can include jpeglib.h.
 * (stdio.h is sufficient on ANSI-conforming systems.)
 * You may also wish to include "jerror.h".
 */

#include "jpeglib.h"

/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */

#include <setjmp.h>



/******************** JPEG COMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to feed data into the JPEG compressor.
 * We present a minimal version that does not worry about refinements such
 * as error recovery (the JPEG code will just exit() if it gets an error).
 */


/*
 * IMAGE DATA FORMATS:
 *
 * The standard input image format is a rectangular array of pixels, with
 * each pixel having the same number of "component" values (color channels).
 * Each pixel row is an array of JSAMPLEs (which typically are unsigned chars).
 * If you are working with color data, then the color values for each pixel
 * must be adjacent in the row; for example, R,G,B,R,G,B,R,G,B,... for 24-bit
 * RGB color.
 *
 * For this example, we'll assume that this data structure matches the way
 * our application has stored the image in memory, so we can just pass a
 * pointer to our image buffer.  In particular, let's say that the image is
 * RGB color and is described by:
 */

char * image_buffer;	/* Points to large array of R,G,B-order data */
int image_height;	/* Number of rows in image */
int image_width;		/* Number of columns in image */

typedef int LONG;  
typedef unsigned int DWORD;  
typedef unsigned short int WORD;  

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER { 
WORD    bfType; 
DWORD   bfSize; 
WORD    bfReserved1; 
WORD    bfReserved2; 
DWORD   bfOffBits; 
} BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{ 
DWORD      biSize; 
LONG        biWidth; 
LONG        biHeight; 
WORD       biPlanes; 
WORD       biBitCount; 
DWORD      biCompression; 
DWORD      biSizeImage; 
LONG        biXPelsPerMeter; 
LONG        biYPelsPerMeter; 
DWORD      biClrUsed; 
DWORD      biClrImportant; 
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

#pragma pack()

void bgr_to_bmpfile(char *bmp_file,char *rgb,int size,int w,int h,int bit)
{
    FILE *fp = fopen(bmp_file,"wb+");
    if(fp == NULL) return;
 
    BITMAPFILEHEADER bmpheader;
    BITMAPINFOHEADER bmpinfo;
 
    bmpheader.bfType = 0x4d42;
    bmpheader.bfReserved1 = 0;
    bmpheader.bfReserved2 = 0;
    bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpheader.bfSize = bmpheader.bfOffBits + size;
 
    bmpinfo.biSize = sizeof(BITMAPINFOHEADER);
    bmpinfo.biWidth = w;
    bmpinfo.biHeight = -h;
    bmpinfo.biPlanes = 1;
    bmpinfo.biBitCount = bit;
    bmpinfo.biCompression = 0;
    bmpinfo.biSizeImage = (w*bit+31)/32*4*h;
    bmpinfo.biXPelsPerMeter = 100;
    bmpinfo.biYPelsPerMeter = 100;
    bmpinfo.biClrUsed = 0;
    bmpinfo.biClrImportant = 0;
         
    fwrite(&bmpheader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&bmpinfo, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(rgb, size, 1, fp);
 
    fclose(fp);
}
int main(int argc,char **argv)
{
  char *bmppath;
  int image_size;
  int rc;
  int i;
  char b;
  if(argc < 2){
    fprintf(stderr,"ERROR:To few commands.\n");
    return 0;
  }
  bmppath = argv[1];

  image_width =1280;
  image_height = 720;
  image_size = image_width*image_height*3;
  image_buffer = (char *)malloc(image_width*image_height*3);
  if(image_buffer == NULL){
    fprintf(stderr,"malloc error\n");
    return 0;
  }

  memset(image_buffer,0,image_size);

  for(i=0;i<image_height*2;i++){
    image_buffer[3*640*i] = 0xff;
  }

  for(i=0;i<(image_size);i+=3){
    b = image_buffer[i];
    image_buffer[i] = image_buffer[i+2];
    image_buffer[i+2] = b;
  }

  bgr_to_bmpfile(bmppath,image_buffer,image_size,1280,720,24);
}




