
#include <linux/kernel.h>
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

#define DEBUG_SHOW 1
#define BUFFERSIZE 4
#define DEVICE_NAME  "max"

int dev_MAJOR=235;

#define DEVICE_ADD 0x10
#define READ_DEV_ADDR ((DEVICE_ADD<<1)+1)
#define WRITE_DEV_ADDR (DEVICE_ADD<<1)

#define I2C_READ_DATA  1
#define I2C_WRITE_DATA 2


#define GPIO_BASE 0xE000A000
#define MASK_DATA_2_LSW 0x00000010
#define DATA_2_RO 0x00000068
#define DIRM_2 0x00000284
#define OEN_2 0x00000288 




unsigned short int wbuf[320000];
static void __iomem *gpio_base;


struct I2C_MSGbuffer
{
	unsigned short len;
	unsigned char addr;
	char buffer[64];
};

void SetSDAOut(void)
{
  writel(readl(gpio_base+DIRM_2)|0x00000001,gpio_base+DIRM_2);
  writel(readl(gpio_base+OEN_2)|0x00000001,gpio_base+OEN_2);
}

void SetSDAInput(void)
{
  writel(readl(gpio_base+DIRM_2)&0xfffffffe,gpio_base+DIRM_2);
}

void SetSCLOut(void)
{
  writel(readl(gpio_base+DIRM_2)|0x00000002,gpio_base+DIRM_2);
  writel(readl(gpio_base+OEN_2)|0x00000002,gpio_base+OEN_2);
}

void SetSCLInput(void)
{
  writel(readl(gpio_base+DIRM_2)&0xfffffffd,gpio_base+DIRM_2);
}

int GetSDAValue(void)
{
	int ret=__raw_readl(gpio_base+DATA_2_RO)&0x00000001;
	if(ret==0)
		return 0;
	else
	    return 1;
}

void SetSDAHigh(void)
{
  writel(0xfffe0001,gpio_base+MASK_DATA_2_LSW);
}

void SetSDALow(void)
{
  writel(0xfffe0000,gpio_base+MASK_DATA_2_LSW);
}

void SetSCLHigh(void)
{
  writel(0xfffd0002,gpio_base+MASK_DATA_2_LSW);
}

void SetSCLLow(void)
{
  writel(0xfffd0000,gpio_base+MASK_DATA_2_LSW);
}

void I2CStart(void)
{
	SetSDAHigh();
	SetSCLHigh();
    udelay(40);
	SetSDALow();
	udelay(100);
	SetSCLLow();
	udelay(80);
}


void I2CStop(void)
{
	SetSDALow();
	udelay(60);
	SetSCLHigh();
	udelay(80);
	SetSDAHigh();
}

int WaitAck(void)
{
	int isum=2000;
	int ret=0;
	udelay(1);
    SetSDAInput();
	SetSCLHigh();
	udelay(10);
	while(isum>0)
	{
		ret=GetSDAValue();
		if(ret==0)
		{
			break;
		}
		udelay(2);
		isum--;
	}
	SetSCLLow();
	SetSDAOut();
	udelay(2);
	return ret;
}

void SendAck(void)
{
	SetSDALow();
	SetSCLHigh();
	udelay(8);
	SetSCLLow();
	SetSDAHigh();
}


 void SendNotAck(void)
 {
	SetSDAHigh();
	SetSCLHigh();
	udelay(8);
	SetSCLLow();
 }


void I2CSendByte(unsigned char ch)
{
    unsigned char i=8;
      while (i--)
      {
		    if(ch&0x80)
		    {
				SetSDAHigh();
		    }
			else
			{
				SetSDALow();
			}
			udelay(10);
			SetSCLHigh();
			udelay(100);
			SetSCLLow();
			udelay(80);
			ch=ch<<1;
      }
	  SetSDAHigh();
}

unsigned char I2CReceiveByte(void)
{
	  int ret=0;
      unsigned char i=8;
	  unsigned char data=0;
      SetSDAHigh();
      SetSDAInput();
	  udelay(20);
      while (i--)
      {
    	    data<<=1;
			SetSCLHigh();
			udelay(60);
			ret=GetSDAValue();
			data|=ret;
			udelay(20);
			SetSCLLow();
			udelay(60);
		}
	  SetSDAOut();
      return data;
}

int writeOnedate(unsigned char addr,unsigned char ch)
{
	int ret=0;
  //printk("write begin\n");
	I2CStart();
	I2CSendByte(WRITE_DEV_ADDR);
	ret=WaitAck();
#if DEBUG_SHOW
	printk("writedate dev addr ACK:%d\n",ret);
#endif
	if(ret==0)
	{
		I2CSendByte(addr);
		ret=WaitAck();
#if DEBUG_SHOW
		printk("writedate data addr 0x%.2x ACK:%d\n",addr,ret);
#endif
		if(ret==0)
		{
			I2CSendByte(ch);
			ret=WaitAck();
#if DEBUG_SHOW
			printk("writedate data 0x%.2x ACK:%d\n",ch,ret);
#endif
		}
	}
	I2CStop();
	mdelay(10);
	return ret;
}

int readOnedate(unsigned char addr,unsigned char*ch)
{
	int ret=0;
  //printk("read begin\n");
	I2CStart();
	I2CSendByte(WRITE_DEV_ADDR);
	ret=WaitAck();
#if DEBUG_SHOW
	printk("readdate dev addr ACK:%d\n",ret);
#endif
	if(ret==0)
	{
		I2CSendByte(addr);
		ret=WaitAck();
#if DEBUG_SHOW
		printk("readdate data addr 0x%.2x ACK:%d\n",addr,ret);
#endif
		if(ret==0)
		{
			//I2CStop();
			I2CStart();
			I2CSendByte(READ_DEV_ADDR);
			ret=WaitAck();
#if DEBUG_SHOW
			printk("readdate dev addr ACK:%d\n",ret);
#endif
			if(ret==0)
			{
				*ch=I2CReceiveByte();
#if DEBUG_SHOW
				printk("readdate data:0x%.2x\n",*ch);
#endif
				SendNotAck();
			}
		}
	}
	I2CStop();
	return ret;
}

int readThreeTimes(unsigned char addr,unsigned char*ch)
{
	int i=3;
	while(i>0)
	{
		if(readOnedate(addr,ch)==0)
		{
			break;
		}
		i--;
	}
	if(i==0)return 0;
	return 1;
}

int writeThreeTimes(unsigned char addr,unsigned char ch)
{
	int i=3;
	while(i>0)
	{
		if(writeOnedate(addr,ch)==0)
		{
			break;
		}
		i--;
	}
	if(i==0)return 0;
	return 1;
}


static int I2C_open(struct inode *inode, struct file *filp)
{
	int ret=0;

	SetSDAOut();
	SetSCLOut();
  printk("#############open##############\n");

	return ret;
}


static int max_read(struct file *filp,char __user *buf,size_t count,loff_t *f_pos)
{
  return 0;
}


static int I2C_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
	case I2C_READ_DATA:
		{
			int i=0;
			struct I2C_MSGbuffer imsg;
      struct I2C_MSGbuffer wmsg;
      memset(&wmsg,0,sizeof(struct I2C_MSGbuffer));
      wmsg.addr=0x03;
      wmsg.len=10;
      wmsg.buffer[0]=0x11;
      wmsg.buffer[1]=0xc0;
      wmsg.buffer[2]=0;
      wmsg.buffer[3]=0;
      wmsg.buffer[4]=0x09;
      wmsg.buffer[5]=0;
      wmsg.buffer[6]=0x06;
      wmsg.buffer[7]=0x33;
      wmsg.buffer[8]=0;
      wmsg.buffer[9]=0x34;
      for(i=0;i<wmsg.len;i++)
      {
        if(writeThreeTimes(wmsg.addr+i,wmsg.buffer[i])==0)
        {
          break;
        }
      }
      if(writeThreeTimes(0x10,0x83)==0)
      {
        break;
      }
      printk("write %d successfully\n",i);
      printk("ioctl read\n");
			if (copy_from_user(&imsg,(char*)arg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			for(i=0;i<imsg.len;i++)
			{
				if(readThreeTimes(imsg.addr+i,&imsg.buffer[i])==0)
				{
					break;
				}
			}
			if (copy_to_user((char*)arg,&imsg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			return i;
		}
		break;
	case I2C_WRITE_DATA:
		{
			int i=0;
			struct I2C_MSGbuffer imsg;
      printk("ioctl write\n");
			if (copy_from_user(&imsg,(char*)arg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			for(i=0;i<imsg.len;i++)
			{
				if(writeThreeTimes(imsg.addr+i,imsg.buffer[i])==0)
				{
					break;
				}
			}
			return i;
		}
		break;
	default:
		break;
	}
	return 0;
}

static int I2C_release(struct inode *inode,struct file *filp)
{
  printk("###################release###############\n");
	return 0;
}


static struct file_operations I2C_fops={
	.owner = THIS_MODULE,
	.open = I2C_open,
  .read = max_read,
	.unlocked_ioctl = I2C_ioctl,
	.release = I2C_release,
};

static int __init I2C_init(void)
{
	int ret;
	ret = register_chrdev(dev_MAJOR,DEVICE_NAME,&I2C_fops);
	if(ret<0)
	{	
		printk("i2c can't get the major number...\n");
		return ret;
	}

	printk("i2c module init...\n");

  gpio_base = ioremap(GPIO_BASE,0x10000);

	if(gpio_base==NULL)
    	{
      	  printk("error in ioremap\n");
    	}
	return 0;
}
 
static void __exit I2C_exit(void)
{
	unregister_chrdev(dev_MAJOR,DEVICE_NAME);
	printk("i2c module exit...\n");
}	  

module_init(I2C_init);
module_exit(I2C_exit);

MODULE_AUTHOR("dooon <tangxu@gmail.com>");
MODULE_DESCRIPTION("max9860 Test Driver");
MODULE_LICENSE("GPL");
