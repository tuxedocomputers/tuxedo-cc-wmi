obj-m += ./src/tuxedo_wmi.o

PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build

# Module build targets
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean


# Package version from dkms.conf
VER := $(shell sed -n 's/^PACKAGE_VERSION=\([^\n]*\)/\1/p' dkms.conf)

# Deb package folder variables
DEB_PACKAGE_BASE := deb/tuxedo-wmi-$(VER)
DEB_PACKAGE_SRC := $(DEB_PACKAGE_BASE)/usr/src/tuxedo-wmi-$(VER)/
DEB_PACKAGE_CTRL := $(DEB_PACKAGE_BASE)/DEBIAN

package: package-deb package-rpm

package-deb:
	# Rename or complete folder structure according to current version
	mv deb/tuxedo-wmi-* $(DEB_PACKAGE_BASE) || true
	mkdir $(DEB_PACKAGE_BASE)/usr/src || true
	mkdir $(DEB_PACKAGE_SRC) || true
	mv $(DEB_PACKAGE_BASE)/usr/src/tuxedo-wmi-* $(DEB_PACKAGE_SRC) || true
	# Replace version numbers in control/script files
	sed -i 's/^Version:[^\n]*/Version: $(VER)/g' $(DEB_PACKAGE_CTRL)/control
	sed -i 's/^version=[^\n]*/version=$(VER)/g' $(DEB_PACKAGE_CTRL)/postinst
	sed -i 's/^version=[^\n]*/version=$(VER)/g' $(DEB_PACKAGE_CTRL)/prerm
	# Copy source
	cp -rf dkms.conf $(DEB_PACKAGE_SRC)
	cp -rf Makefile $(DEB_PACKAGE_SRC)
	cp -rf src $(DEB_PACKAGE_SRC)
	# Make sure control folder has acceptable permissions
	chmod -R 755 $(DEB_PACKAGE_CTRL)
	# Make deb package
	dpkg-deb --root-owner-group -b $(DEB_PACKAGE_BASE) tuxedo-wmi-$(VER).deb


RPM_PACKAGE_SRC := rpm/SOURCES/tuxedo-wmi-$(VER)
RELEASE := 0

package-rpm:
	# Create folder source structure according to current version
	rm -rf rpm/SOURCES || true
	mkdir rpm/SOURCES
	mkdir $(RPM_PACKAGE_SRC)
	# Modify spec file with version etc.
	sed -i 's/^Version:[^\n]*/Version:        $(VER)/g' rpm/SPECS/tuxedo-wmi.spec
	sed -i 's/^Release:[^\n]*/Release:        $(RELEASE)/g' rpm/SPECS/tuxedo-wmi.spec
	# Copy source
	cp -rf dkms.conf $(RPM_PACKAGE_SRC)
	cp -rf Makefile $(RPM_PACKAGE_SRC)
	cp -rf src $(RPM_PACKAGE_SRC)
	cp -rf LICENSE $(RPM_PACKAGE_SRC)
	# Compress/package source
	cd rpm/SOURCES && tar cjvf tuxedo-wmi-$(VER).tar.bz2 tuxedo-wmi-$(VER)
	# Make rpm package
	rpmbuild --debug -bb --define "_topdir `pwd`/rpm" rpm/SPECS/tuxedo-wmi.spec

package-rpm-clean:
	rm -rf rpm/BUILD || true
	rm -rf rpm/SOURCES || true
	rm -rf rpm/RPMS || true
	rm -rf rpm/SRPMS || true
	rm -rf rpm/BUILDROOT || true
