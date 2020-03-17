
obj-m := ./src/tuxedo_wmi.o

PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build

# Module build targets
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean


# --------------
# Packaging only
# ---------------

# Package version from dkms.conf
VER := $(shell sed -n 's/^PACKAGE_VERSION=\([^\n]*\)/\1/p' dkms.conf)
MODULE_NAME := $(shell sed -n 's/^PACKAGE_NAME=\([^\n]*\)/\1/p' dkms.conf)

DEB_PACKAGE_NAME := $(MODULE_NAME)-$(VER)

# Deb package folder variables
DEB_PACKAGE_BASE := deb/$(DEB_PACKAGE_NAME)
DEB_PACKAGE_SRC := $(DEB_PACKAGE_BASE)/usr/src/$(DEB_PACKAGE_NAME)
DEB_PACKAGE_CTRL := $(DEB_PACKAGE_BASE)/DEBIAN

package: package-deb package-rpm
package-clean: package-deb-clean package-rpm-clean

package-deb:
	# Create/complete folder structure according to current version
	rm -rf $(DEB_PACKAGE_BASE) || true
	cp -rf deb/module-name $(DEB_PACKAGE_BASE)
	mv $(DEB_PACKAGE_BASE)/usr/share/doc/module-name $(DEB_PACKAGE_BASE)/usr/share/doc/$(MODULE_NAME)
	mkdir -p $(DEB_PACKAGE_BASE)/usr/src || true
	mkdir -p $(DEB_PACKAGE_SRC) || true
	mkdir -p $(DEB_PACKAGE_BASE)/usr/share/$(MODULE_NAME) || true

	# Replace name/version numbers in control/script files
	sed -i 's/^Version:[^\n]*/Version: $(VER)/g' $(DEB_PACKAGE_CTRL)/control
	sed -i 's/^Package:[^\n]*/Package: $(MODULE_NAME)/g' $(DEB_PACKAGE_CTRL)/control
	sed -i 's/^version=[^\n]*/version=$(VER)/g' $(DEB_PACKAGE_CTRL)/postinst
	sed -i 's/^module=[^\n]*/module=$(MODULE_NAME)/g' $(DEB_PACKAGE_CTRL)/postinst
	sed -i 's/^version=[^\n]*/version=$(VER)/g' $(DEB_PACKAGE_CTRL)/prerm
	sed -i 's/^module=[^\n]*/module=$(MODULE_NAME)/g' $(DEB_PACKAGE_CTRL)/prerm
	# Copy source
	cp -rf dkms.conf $(DEB_PACKAGE_SRC)
	cp -rf Makefile $(DEB_PACKAGE_SRC)
	cp -rf src $(DEB_PACKAGE_SRC)
	cp -rf src_pkg/dkms_postinst $(DEB_PACKAGE_BASE)/usr/share/$(MODULE_NAME)/postinst
	# Make sure control folder has acceptable permissions
	chmod -R 755 $(DEB_PACKAGE_CTRL)
	# Make deb package
	dpkg-deb --root-owner-group -b $(DEB_PACKAGE_BASE) $(DEB_PACKAGE_NAME).deb

package-deb-clean:
	rm -rf deb/$(MODULE_NAME)-* > /dev/null 2>&1 || true
	rm *.deb > /dev/null 2>&1 || true

RPM_PACKAGE_NAME := $(MODULE_NAME)-$(VER)
RPM_PACKAGE_SRC := rpm/SOURCES/$(RPM_PACKAGE_NAME)
RPM_SPEC := rpm/SPECS/$(MODULE_NAME).spec
RELEASE := 0

package-rpm:
	# Create folder source structure according to current version
	rm -rf rpm || true
	mkdir -p $(RPM_PACKAGE_SRC)
	mkdir -p rpm/SPECS
	# Copy spec template
	cp -rf src_pkg/rpm_pkg.spec $(RPM_SPEC)
	# Modify spec file with version etc.
	sed -i 's/^%define module[^\n]*/%define module $(MODULE_NAME)/g' $(RPM_SPEC)
	sed -i 's/^Version:[^\n]*/Version:        $(VER)/g' $(RPM_SPEC)
	sed -i 's/^Release:[^\n]*/Release:        $(RELEASE)/g' $(RPM_SPEC)
	# Copy source
	cp -rf dkms.conf $(RPM_PACKAGE_SRC)
	cp -rf Makefile $(RPM_PACKAGE_SRC)
	cp -rf src $(RPM_PACKAGE_SRC)
	cp -rf LICENSE $(RPM_PACKAGE_SRC)
	cp -rf src_pkg/dkms_postinst $(RPM_PACKAGE_SRC)/postinst
	# Compress/package source
	cd rpm/SOURCES && tar cjvf $(RPM_PACKAGE_NAME).tar.bz2 $(RPM_PACKAGE_NAME)
	# Make rpm package
	rpmbuild --debug -bb --define "_topdir `pwd`/rpm" $(RPM_SPEC)
	# Copy built package
	cp rpm/RPMS/noarch/*.rpm .

package-rpm-clean:
	rm -rf rpm > /dev/null 2>&1 || true
	rm *.rpm > /dev/null 2>&1 || true
