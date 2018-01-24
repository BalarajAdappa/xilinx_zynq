/*************************************************************************
    > File Name: test.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2017年10月15日 星期日 09时29分52秒
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(void)
{
  int fd;
  char buf1[1]={0};
  char buf2[1]={1};
  fd = open("/dev/led",O_RDWR);
  if(fd<0)
  {
    printf("open led failed\n");
    return -1;
  }
  while(1)
  {
    ioctl(fd,0,1);
    sleep(1);
    ioctl(fd,0,0);
    sleep(1);
  }
  close(fd);
  return 0;
}
