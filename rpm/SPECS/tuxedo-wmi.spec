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
Version:        x.x.x
Release:        x
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

%clean
rm -rf %{buildroot}

%files
%defattr(0644,root,root,0755)
%attr(0755,root,root) /usr/src/%{module}-%{version}/
%attr(0644,root,root) /usr/src/%{module}-%{version}/*
%attr(0755,root,root) /usr/src/%{module}-%{version}/src/
%attr(0644,root,root) /usr/src/%{module}-%{version}/src/*
%license LICENSE

%post
occurrences=/usr/sbin/dkms status | grep "%{module}" | grep "%{version}" | wc -l
if [ ! occurrences > 0 ];
then
    /usr/sbin/dkms add -m %{module} -v %{version}
fi
/usr/sbin/dkms build -m %{module} -v %{version}
/usr/sbin/dkms install -m %{module} -v %{version}
/usr/sbin/rmmod -s tuxedo_wmi
/usr/sbin/modprobe tuxedo_wmi
exit 0

%preun
/usr/sbin/rmmod tuxedo_wmi
/usr/sbin/dkms remove -m %{module} -v %{version} --all
exit 0

%changelog
* Thu Mar 05 2020 Eckhart Mohr <tux@tuxedocomputers.com> 1.0.0-0
- Init
