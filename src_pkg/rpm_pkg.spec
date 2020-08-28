%define module module-name

#
# spec file for package tuxedo-cc-wmi
#
# Copyright (c) 2019 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


Summary:        An interface to WMI methods/control on TUXEDO Laptops
Name:           %{module}
Version:        x.x.x
Release:        x
License:        GPLv3+
Group:          Hardware/Other
BuildArch:      noarch
Url:            https://www.tuxedocomputers.com
Source:         %{module}-%{version}.tar.bz2
Requires:       dkms >= 1.95
Obsoletes:      tuxedo-wmi >= 2.0.0, tuxedo-wmi <= 2.0.3
BuildRoot:      %{_tmppath}
Packager:       Tomte <tux@tuxedocomputers.com>

%description
This module provides an interface to controlling various functionality (mainly
connected to the EC) through WMI.

%prep
%setup -n %{module}-%{version} -q

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/src/%{module}-%{version}/
cp dkms.conf Makefile %{buildroot}/usr/src/%{module}-%{version}
cp -R src/ %{buildroot}/usr/src/%{module}-%{version}
mkdir -p %{buildroot}/usr/share/
mkdir -p %{buildroot}/usr/share/%{module}/
cp postinst %{buildroot}/usr/share/%{module}

%clean
rm -rf %{buildroot}

%files
%defattr(0644,root,root,0755)
%attr(0755,root,root) /usr/src/%{module}-%{version}/
%attr(0644,root,root) /usr/src/%{module}-%{version}/*
%attr(0755,root,root) /usr/src/%{module}-%{version}/src/
%attr(0644,root,root) /usr/src/%{module}-%{version}/src/*
%attr(0755,root,root) /usr/share/%{module}/
%attr(0755,root,root) /usr/share/%{module}/postinst
%license LICENSE

%post
for POSTINST in /usr/lib/dkms/common.postinst /usr/share/%{module}/postinst; do
    if [ -f $POSTINST ]; then
        $POSTINST %{module} %{version} /usr/share/%{module}
        RET=$?

        # Attempt to (re-)load module immediately, fail silently if not possible at this stage

        # Also stop tccd service if running before
        echo "Check tccd running status"
        if systemctl is-active --quiet tccd.service; then
            TCCD_RUNNING=true
        else
            TCCD_RUNNING=false
        fi

        if $TCCD_RUNNING; then
            echo "Stop tccd temporarily"
            systemctl stop tccd 2>&1 || true
        fi

        echo "(Re)load tuxedo-cc-wmi module if possible"
        rmmod %{module} > /dev/null 2>&1 || true
        if modprobe %{module} > /dev/null 2>&1; then
            echo "(Re)load complete"
        fi

        # Restart tccd after reload if it was running
        if $TCCD_RUNNING; then
            echo "Start tccd again"
            systemctl start tccd 2>&1 || true
        fi

        exit $RET
    fi
    echo "WARNING: $POSTINST does not exist."
done

echo -e "ERROR: DKMS version is too old and %{module} was not"
echo -e "built with legacy DKMS support."
echo -e "You must either rebuild %{module} with legacy postinst"
echo -e "support or upgrade DKMS to a more current version."
exit 1


%preun
echo -e
echo -e "Uninstall of %{module} module (version %{version}-%{release}) beginning:"
dkms remove -m %{module} -v %{version} --all --rpm_safe_upgrade
if [ $1 != 1 ];then
    /usr/sbin/rmmod %{module} > /dev/null 2>&1 || true
fi
exit 0


%changelog
* Fri Aug 28 2020 C Sandberg <tux@tuxedocomputers.com> 0.1.6-1
- Fixed webcam status issue on some cl devices
* Mon Aug 10 2020 C Sandberg <tux@tuxedocomputers.com> 0.1.5-1
- Fixed fan speed write timing for most cl devices
- Added webcam set interface support for older cl devices
* Thu Jun 18 2020 C Sandberg <tux@tuxedocomputers.com> 0.1.4-1
- Added device support
* Wed Apr 8 2020 C Sandberg <tux@tuxedocomputers.com> 0.1.3-1
- Add package dependencies on old named package
* Mon Mar 31 2020 C Sandberg <tux@tuxedocomputers.com> 0.1.2-0
- Fixed missing reference to tuxedo_wmi (module class)
* Mon Mar 30 2020 C Sandberg <tux@tuxedocomputers.com> 0.1.1-0
- Added module version getter to ioctl api
- Revised ioctl api
* Fri Mar 27 2020 C Sandberg <tux@tuxedocomputers.com> 0.1.0-0
- Module/package reborn as tuxedo-cc-wmi
