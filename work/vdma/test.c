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

#define VDMA_RX 1
#define BUF_SIZE (1280*720*3)

static char *buf;
int main(int argc,char **argv)
{
  unsigned int fd;
//  unsigned int fd0;
  FILE *fp_in;
  FILE *fp_out;
  int rc;
  if(argc<3){
    fprintf(stderr,"Error: Too few command line arguments.\n");
    return -EINVAL;
  }
  fd = open("/dev/vdma",O_RDWR);
  if(!fd){
    printf("opening device error\n");
    return 0;
  }
  if((fp_in = fopen(argv[1],"rb")) == NULL){
    fprintf(stderr,"cant open %s\n",argv[1]);
    return 0;
  }

  buf = (char *)malloc(BUF_SIZE);
  if(buf==NULL){
    fprintf(stderr,"cant malloc buf\n");
    return -EINVAL;
  }
  rc = fread(buf,1,BUF_SIZE,fp_in);
  if(rc < 0){
    fprintf(stderr,"fread error\n");
    return 0;
  }
  write(fd,buf,BUF_SIZE);
  ioctl(fd,VDMA_RX,buf);
  //fd0 = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IRGRP|S_IWGRP|S_IROTH);
  //if(fd0<0){
  //  perror("Error opening output file.");
  //  return 0;
  //}
  if((fp_out = fopen(argv[2],"wb"))==NULL){
    fprintf(stderr,"cant ioen %s\n",argv[2]);
    return 0;
    }
  //rc = write(fd0,buf,BUF_SIZE);
  rc = fwrite(buf,1,BUF_SIZE,fp_out);
  //close(fd0);
  fclose(fp_in);
  fclose(fp_out);
  close(fd);
}
