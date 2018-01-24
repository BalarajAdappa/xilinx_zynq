ifeq ($(KERNELRELEASE),)

KERNELDIR ?=/home/osrc/xilinx/04-Linux_Source/Kernel/linux-xlnx

PWD :=$(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR)=$(PWD) module_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .temp_versions

.PHONY: modules modules_install clean

else
	obj-m :=hello_module.o
endif
