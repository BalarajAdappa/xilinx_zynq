/*************************************************************************
    > File Name: led.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2017年10月14日 星期六 16时02分00秒
 ************************************************************************/

#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/types.h>
#include<linux/errno.h>
#include<linux/init.h>
#include<linux/cdev.h>
#include<linux/mm.h>
#include<linux/uaccess.h>
#include<linux/slab.h>
#include<linux/fcntl.h>
#include<linux/vmalloc.h>
#include <linux/version.h>
#include <linux/ctype.h>
#include <linux/pagemap.h>
//#include <mach/map.h>·····
//#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
MODULE_LICENSE("dual BSD/GPL");



#define GPIO_BASE 0xE000A000
#define DIRM_3 0x000002C4
#define OEN_3 0x000002C8 
#define MASK_DATA_3_MSW 0x0000001C

#define LED_MAJOR 235
#define LED_NAME "led"

static void __iomem *gpio_base;

//MIO_led   gpio (60~63)
static int led_open(struct inode *inode,struct file *filp)
{
  unsigned dirm;
  unsigned oen;
  dirm = readl(gpio_base + DIRM_3);
  oen = readl(gpio_base + OEN_3);
  writel(dirm|0xf0000000,gpio_base + DIRM_3);
  writel(oen|0xf0000000,gpio_base + OEN_3);
  printk("DIRM_3 0x%x\n",readl(gpio_base + DIRM_3));
  printk("ONE_3 0x%x\n",readl(gpio_base + OEN_3));
  return 0;
}
static int led_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_fpos)
{
  char wbuf[1];
  printk("write begin\n");
  if(copy_from_user(wbuf,buf,count))
    return -EFAULT;
if(wbuf[0]==0)
  writel(0x1fffffff&0xfffff000,gpio_base + MASK_DATA_3_MSW);
else if(wbuf[0]==1)
  writel(0x1fffffff&0xffff0000,gpio_base + MASK_DATA_3_MSW);
return 0;
}
static long led_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
  switch(cmd)
  {
  case 0:
    if(arg==1)
      writel(0xefffffff&0xffff0000,gpio_base + MASK_DATA_3_MSW);
    else if(arg==0)
      writel(0xefffffff&0xfffff000,gpio_base + MASK_DATA_3_MSW);
    return 0;
  case 1:
    if(arg==1)
      writel(0xdfffffff&0xffff0000,gpio_base + MASK_DATA_3_MSW);
    else if(arg==0)
      writel(0xdfffffff&0xfffff000,gpio_base + MASK_DATA_3_MSW);
    return 0;
  case 2:
    if(arg==1)
      writel(0xbfffffff&0xffff0000,gpio_base + MASK_DATA_3_MSW);
    else if(arg==0)
      writel(0xbfffffff&0xfffff000,gpio_base + MASK_DATA_3_MSW);
    return 0;
  case 3:
    if(arg==1)
      writel(0x7fffffff&0xffff0000,gpio_base + MASK_DATA_3_MSW);
    else if(arg==0)
      writel(0x7fffffff&0xfffff000,gpio_base + MASK_DATA_3_MSW);
    return 0;
  default:
    return -EINVAL;
  }
}
int led_release(struct inode *inode,struct file *filp)
{
  printk("release\n");
  return 0;
}
struct file_operations led_fops =
{
  .owner = THIS_MODULE,
  .open = led_open,
  .write = led_write,
  .release = led_release,
  .unlocked_ioctl = led_ioctl,
};
static int __init led_init(void)
{
  int ret;
  ret = register_chrdev(LED_MAJOR,LED_NAME,&led_fops);
  if(ret<0)
  {	
     printk("led can't get the major number...\n");
     return ret;
  }
  gpio_base = ioremap(GPIO_BASE,0x10000);
  printk("led module init...\n");
  return 0;
}
static void __exit led_exit(void)
{
  unregister_chrdev(LED_MAJOR,LED_NAME);
  printk("module exit\n");
}
module_init(led_init);
module_exit(led_exit);
