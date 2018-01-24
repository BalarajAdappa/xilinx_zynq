/************************************************************************
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

#define VDMA_RX 1
#define WHIDE 1280
#define HEIGHT 1024
#define BUF_SIZE (WHIDE*HEIGHT*3)

static char *buf;
int main(int argc,char **argv)
{
  unsigned int fd;
  FILE *pFrame1;
  FILE *pFrame2;
  FILE *pFrame3;
  int rc;

  fd = open("/dev/vdma",O_RDWR);
  if(!fd){
    printf("opening device error\n");
    return 0;
  }

  buf = (char *)malloc(BUF_SIZE*3);
  if(buf==NULL){
    fprintf(stderr,"cant malloc buf\n");
    return -EINVAL;
  }
  ioctl(fd,VDMA_RX,buf);

  if((pFrame1 = fopen("pFrame1.rgb","wb"))==NULL){
    fprintf(stderr,"cant openfile");
    return 0;
    }
  if((pFrame2 = fopen("pFrame2.rgb","wb"))==NULL){
    fprintf(stderr,"cant openfile");
    return -EINVAL;
  }
    if((pFrame3 = fopen("pFrame3.rgb","wb"))==NULL){
      fprintf(stderr,"cant openfile");
      return -EINVAL;
    }

  rc = fwrite(buf,1,BUF_SIZE,pFrame1);
  rc = fwrite(buf+BUF_SIZE,1,BUF_SIZE,pFrame2);
  rc = fwrite(buf+2*BUF_SIZE,1,BUF_SIZE,pFrame3);
  fclose(pFrame1);
  fclose(pFrame2);
  fclose(pFrame3);
  close(fd);
}
