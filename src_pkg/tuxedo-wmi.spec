%define module tuxedo-wmi

#
# spec file for package tuxedo-keyboard
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


Summary:        interface to WMI methods/control on TUXEDO Laptops
Name:           %{module}
Version:        2.0.3
Release:        0
License:        GPL-3.0+
Group:          Hardware/Other
BuildArch:      noarch
Url:            https://gitlab.com/tuxedocomputers/development/tuxedo-keyboard
Source:         %{module}-%{version}.tar.bz2
Provides:	tuxedo-wmi = %{version}-%{release}
Obsoletes:	tuxedo-wmi < %{version}-%{release}
Requires:       dkms >= 1.95
BuildRoot:      %{_tmppath}

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
        modprobe %{module} > /dev/null 2>&1 || true
        exit $?
    fi
    echo "WARNING: $POSTINST does not exist."
done

echo -e "ERROR: DKMS version is too old and tuxedo-wmi was not"
echo -e "built with legacy DKMS support."
echo -e "You must either rebuild tuxedo-wmi with legacy postinst"
echo -e "support or upgrade DKMS to a more current version."
exit 1


%preun
echo -e
echo -e "Uninstall of tuxedo-wmi module (version 1.0.0) beginning:"
dkms remove -m %{module} -v %{version} --all --rpm_safe_upgrade
if [ $1 != 1 ];then
    /usr/sbin/rmmod tuxedo_wmi > /dev/null 2>&1 || true
fi
exit 0


%changelog
* Thu Mar 05 2020 Eckhart Mohr <tux@tuxedocomputers.com> 1.0.0-0
- Init
