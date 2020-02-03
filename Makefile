obj-m += ./src/tuxedo_wmi.o

tuxedo_tuxedo-objs := ./src/tuxedo_wmi.o

PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build

# Module build targets
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean


# Package build targets
PACKAGE_BUILD_DIR := package_build/

package:
	make clean
	mkdir $(PACKAGE_BUILD_DIR) || true
	cp -rf dkms.conf $(PACKAGE_BUILD_DIR)
	cp -rf Makefile $(PACKAGE_BUILD_DIR)
	cp -rf src $(PACKAGE_BUILD_DIR)
	cd $(PACKAGE_BUILD_DIR) && dkms mkdeb . --source-only

clean-package:
	rm -rf $(PACKAGE_BUILD_DIR)
	rm -rf *.deb
