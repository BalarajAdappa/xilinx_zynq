/************************************************************************
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
#define HEIGHT 720
#define STRIDE 1920
#define BUF_SIZE (WHIDE*HEIGHT*3)
#define MAX_SIZE (1920*1080*3)
static void __iomem *vdma_base;
static void __iomem *gpio_base;

int dev_major = 235;

char *VIDEO_BASSADDR;
unsigned int src_phys;

static int vdma_open(struct inode *inode,struct file *file)
{
  //set hdmi_phd high
  writel(readl(gpio_base+DIRM_2)|0x00000001,gpio_base+DIRM_2);
  writel(readl(gpio_base+OEN_2)|0x00000001,gpio_base+OEN_2);
  writel(0xfffe0001,gpio_base+MASK_DATA_2_LSW);
  printk("hdmi connect.\n");
  mdelay(1000);
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
  arg_ptr = (void __user *)arg;

  switch(cmd){
  case VDMA_RX:
    //ddr in
    writel(0x00000003,(vdma_base+0x030));
    writel(src_phys,(vdma_base+0x0ac));
    //writel(pFrames_phys[1],(vdma_base+0x0b0));
    //writel(pFrames_phys[2],(vdma_base+0x0b4));
    writel((STRIDE*3),(vdma_base+0x0a8));
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
    if(copy_to_user(arg_ptr,VIDEO_BASSADDR,MAX_SIZE) !=0){
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
  VIDEO_BASSADDR = (char *)dma_alloc_writecombine(NULL, MAX_SIZE, &src_phys, GFP_KERNEL);
  if(NULL==VIDEO_BASSADDR){
    printk(KERN_ALERT "can't alloc buffer for VIDEO_BASEADDR\n");
    return -ENOMEM;
  }
  memset(VIDEO_BASSADDR, 0x00, (MAX_SIZE));
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
  unregister_chrdev(dev_major,DEVICE_NAME);
  printk("vdma exit\n");
}

module_init(vdma_init);
module_exit(vdma_exit);

MODULE_AUTHOR("dooon <tangxu@gmail.com>");
MODULE_DESCRIPTION("xilinx vdma Test Driver");
MODULE_LICENSE("GPL");
