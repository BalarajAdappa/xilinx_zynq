/*************************************************************************
    > File Name: vdma.h
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2018年01月04日 星期四 10时08分49秒
 ************************************************************************/

#define DEVICE_NAME "vdma"

#define VDMA_BASE 0X43000000
#define GPIO_BASE 0XE000A000

#define MASK_DATA_2_LSW 0x00000010
#define DIRM_2 0x00000284
#define OEN_2 0x00000288

#define MEMDEV_IOC_MAGIC  'k'
#define MEMDEV_IOC_MAXNR 8
