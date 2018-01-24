/*************************************************************************
    > File Name: test.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2017年12月28日 星期四 15时05分13秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <setjmp.h>

#define VDMA_RX 1
#define WHIDE 1920
#define HEIGHT 1080
#define BUF_SIZE (WHIDE*HEIGHT*3)
#define MAX_SIZE (1920*1080*3)

typedef long LONG;
typedef unsigned int DWORD;
typedef unsigned short int WORD;


#pragma pack(1)
typedef struct tagBITMAPFILEHEADER { 
  WORD    bfType; 
  DWORD   bfSize; 
  WORD    bfReserved1; 
  WORD    bfReserved2; 
  DWORD   bfOffBits; 
} BITMAPFILEHEADER;

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
} BITMAPINFOHEADER;

#pragma pack()

static char *buf;

void bgr_to_bmpfile(char *rgb,int size,int w,int h,int bit)
{
    FILE *fp = fopen("pFrame.bmp","wb+");
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
  unsigned int fd;
  FILE *pFrame1;
  int rc;
  int i;
  char b;
  fd = open("/dev/vdma",O_RDWR);
  if(!fd){
    printf("opening device error\n");
    return 0;
  }

  buf = (char *)malloc(MAX_SIZE);
  if(buf==NULL){
    fprintf(stderr,"cant malloc buf\n");
    return -EINVAL;
  }
  ioctl(fd,VDMA_RX,buf);

  if((pFrame1 = fopen("pFrame.rgb","wb"))==NULL){
    fprintf(stderr,"cant openfile");
    return 0;
    }

  rc = fwrite(buf,1,MAX_SIZE,pFrame1);

  fclose(pFrame1);
  for(i=0;i<MAX_SIZE;i+=3){
    b = buf[i];
    buf[i] = buf[i+2];
    buf[i+2] = b;
  }
  bgr_to_bmpfile(buf,MAX_SIZE,WHIDE,HEIGHT,24);

  close(fd);
}
