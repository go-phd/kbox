obj-m := kbox.o
kbox-objs := kbox_main.o kbox_notifier.o kbox_output.o kbox_console.o kbox_collect.o kbox_netlink.o kbox_cdev.o kbox_monitor.o kbox_ram_image.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

#PWD       := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) clean
