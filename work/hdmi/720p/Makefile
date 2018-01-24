ifeq ($(KERNELRELEASE),)

KERNELDIR ?=/mnt/workspace/linux/kernel
export CROSS_COMPILE=arm-xilinx-linux-gnueabi-
export ARCH=arm
PWD :=$(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR)=$(PWD) module_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .temp_versions

.PHONY: modules modules_install clean

else
	obj-m :=vdma.o
endif
