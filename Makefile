obj-m += ./src/tuxedo_wmi.o

tuxedo_tuxedo-objs := ./src/tuxedo_wmi.o

PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
