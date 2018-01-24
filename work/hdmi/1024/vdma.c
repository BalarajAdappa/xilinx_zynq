/*************************************************************************
    > File Name: vdma.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2017年12月28日 星期四 09时07分11秒
 ************************************************************************/

#include <linux/init.h>
#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include "vdma.h"

#define VDMA_RX 1
#define WHIDE 1280
#define HEIGHT 1024
#define BUF_SIZE (WHIDE*HEIGHT*3)
static void __iomem *vdma_base;
static void __iomem *gpio_base;

int dev_major = 235;

char *VIDEO_BASSADDR;
unsigned int src_phys;
u32 pFrames_phys[3];
char *pFrames[3];

static int vdma_open(struct inode *inode,struct file *file)
{
  //set hdmi_phd high
  writel(readl(gpio_base+DIRM_2)|0x00000001,gpio_base+DIRM_2);
  writel(readl(gpio_base+OEN_2)|0x00000001,gpio_base+OEN_2);
  writel(0xfffe0001,gpio_base+MASK_DATA_2_LSW);
  printk("hdmi connect.\n");
  return 0;
}
//static int vdma_write(struct file *filp,char __user *buf,size_t count,loff_t *f_pos)
static ssize_t vdma_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  if(copy_from_user(VIDEO_BASSADDR,buff,count))
    return -EFAULT;
  return count;
}
static long vdma_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
  void *__user arg_ptr;
  int i;
  arg_ptr = (void __user *)arg;
  for(i=0;i<3;i++){
    pFrames_phys[i] = src_phys+(i*BUF_SIZE);
    pFrames[i] = VIDEO_BASSADDR+(i*BUF_SIZE);
  }

  switch(cmd){
  case VDMA_RX:
    //ddr in
    writel(0x00000003,(vdma_base+0x030));
    writel(pFrames_phys[0],(vdma_base+0x0ac));
    writel(pFrames_phys[1],(vdma_base+0x0b0));
    writel(pFrames_phys[2],(vdma_base+0x0b4));
    writel((WHIDE*3),(vdma_base+0x0a8));
    writel((WHIDE*3),(vdma_base+0x0a4));
    writel(HEIGHT,(vdma_base+0x0a0));
    //ddr out
    //writel(0x00000003,(vdma_base+0x000));
    //writel(src_phys,(vdma_base+0x05c));
    //writel(&buf1[0],vdma_base+0x060);
    //writel(&buf2[0],vdma_base+0x064);
    //writel((1280*3),(vdma_base+0x058));
    //writel((1280*3),(vdma_base+0x054));
    //writel(720,(vdma_base+0x050));
    mdelay(100);
    if(copy_to_user(arg_ptr,pFrames[0],(BUF_SIZE*3)) !=0){
      printk("Unable to copy to userspace\n");
      return -EFAULT; 
    }
    break;
  default:
    return -ENOTTY;
  }
  return 0;
}
static int vdma_release(struct inode *inode,struct file *filp)
{
  writel(0x00000004,(vdma_base+0x030));//stop vdma
  printk("release\n");
  return 0;
}

static struct file_operations vdma_fops={
  .owner = THIS_MODULE,
  .open = vdma_open,
  .write = vdma_write,
  .unlocked_ioctl = vdma_ioctl,
  .release = vdma_release,
};

static int __init vdma_init(void)
{
  int ret;
  VIDEO_BASSADDR = (char *)dma_alloc_writecombine(NULL, (BUF_SIZE*3), &src_phys, GFP_KERNEL);
  if(NULL==VIDEO_BASSADDR){
    printk(KERN_ALERT "can't alloc buffer for VIDEO_BASEADDR\n");
    return -ENOMEM;
  }
  memset(VIDEO_BASSADDR, 0x00, (BUF_SIZE*3));
  ret = register_chrdev(dev_major,DEVICE_NAME,&vdma_fops);
  if(ret<0){
    printk("vdma cant get the major number\n");
    return ret;
  }
  gpio_base = ioremap(GPIO_BASE,0x10000);
  vdma_base = ioremap(VDMA_BASE,0x10000);
  if((vdma_base==NULL)){
    printk("error in ioremap\n");
    return 0;
  }
  return 0;
}
static void __exit vdma_exit(void)
{
  dma_free_writecombine(NULL, BUF_SIZE*3, VIDEO_BASSADDR, src_phys);
  unregister_chrdev(dev_major,DEVICE_NAME);
  printk("vdma exit\n");
  release_mem_region(GPIO_BASE,0x10000);
  iounmap(gpio_base);
  release_mem_region(VDMA_BASE,0x10000);
    iounmap(vdma_base);
}

module_init(vdma_init);
module_exit(vdma_exit);

MODULE_AUTHOR("dooon <tangxu@gmail.com>");
MODULE_DESCRIPTION("xilinx vdma Test Driver");
MODULE_LICENSE("GPL");
