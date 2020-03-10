obj-m += ./src/tuxedo_wmi.o

tuxedo_tuxedo-objs := ./src/tuxedo_wmi.o

PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build

# Module build targets
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean


# Package version from dkms.conf
VER := $(shell sed -n 's/^PACKAGE_VERSION=\([^\n]*\)/\1/p' dkms.conf)

DEB_PACKAGE_SRC := deb/tuxedo-wmi-$(VER)/usr/src/tuxedo-wmi-$(VER)/
DEB_PACKAGE_CTRL := deb/tuxedo-wmi-$(VER)/DEBIAN

package: package-deb

package-deb:
	mv deb/tuxedo-wmi-* deb/tuxedo-wmi-$(VER) || true
	mkdir deb/tuxedo-wmi-$(VER)/usr/src || true
	mkdir deb/tuxedo-wmi-$(VER)/usr/src/tuxedo-wmi-$(VER) || true
	mv deb/tuxedo-wmi-$(VER)/usr/src/tuxedo-wmi-* deb/tuxedo-wmi-$(VER)/usr/src/tuxedo-wmi-$(VER) || true
	sed -i 's/^Version:[^\n]*/Version: $(VER)/g' $(DEB_PACKAGE_CTRL)/control
	sed -i 's/^version=[^\n]*/version=$(VER)/g' $(DEB_PACKAGE_CTRL)/postinst
	sed -i 's/^version=[^\n]*/version=$(VER)/g' $(DEB_PACKAGE_CTRL)/prerm
	cp -rf dkms.conf $(DEB_PACKAGE_SRC)
	cp -rf Makefile $(DEB_PACKAGE_SRC)
	cp -rf src $(DEB_PACKAGE_SRC)
	dpkg-deb --root-owner-group -b deb/tuxedo-wmi-$(VER) tuxedo-wmi-$(VER).deb
