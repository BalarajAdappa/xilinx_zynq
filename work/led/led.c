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

#define LED_MAJOR 0
#define LED_NAME "led"

static void __iomem *gpio_base;
static int led_major = LED_MAJOR;
struct cdev *led_cdev;
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
};
static int led_init(void)
{
  int ret =0;
  int err =0;
  dev_t devno = MKDEV(led_major,0);
  if(led_major)
  {
    ret = register_chrdev_region(devno,1,LED_NAME);
  }
  else{
    ret = alloc_chrdev_region(&devno,0,1,LED_NAME);
    if(ret<0)
      printk("register_chdev_region faild!\n");
    goto failure_register_chrdev;
  }
  led_major = MAJOR(devno);
  printk("led namme is: %s!\n",LED_NAME);
  printk("led_major = %d\n",led_major);
  led_cdev = kmalloc(sizeof(struct cdev),GFP_KERNEL);
  if(!led_cdev)
  {
    ret = -ENOMEM;
    goto fail_kmalloc;
  }
  memset(led_cdev,0,sizeof(struct cdev));
  cdev_init(led_cdev,&led_fops);
  led_cdev->owner = THIS_MODULE;
  led_cdev->ops = &led_fops;
  err = cdev_add(led_cdev,devno,1);
  if(err)
    printk(KERN_NOTICE "Error in cdev_add\n");
  gpio_base = ioremap(GPIO_BASE,0x10000);
  return 0;
failure_register_chrdev:
  return ret;
fail_kmalloc:
  return ret;
}
static void led_exit(void)
{
  cdev_del(led_cdev);
  kfree(led_cdev);
  unregister_chrdev_region(MKDEV(led_major,0),1);
  printk("module exit\n");
}
module_init(led_init);
module_exit(led_exit);
