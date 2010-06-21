%define module_api %(qore --latest-module-api 2>/dev/null)
%define module_dir %{_libdir}/qore-modules

%if 0%{?sles_version}

%define dist .sles%{?sles_version}

%else
%if 0%{?suse_version}

# get *suse release major version
%define os_maj %(echo %suse_version|rev|cut -b3-|rev)
# get *suse release minor version without trailing zeros
%define os_min %(echo %suse_version|rev|cut -b-2|rev|sed s/0*$//)

%if %suse_version > 1010
%define dist .opensuse%{os_maj}_%{os_min}
%else
%define dist .suse%{os_maj}_%{os_min}
%endif

%endif
%endif

# see if we can determine the distribution type
%if 0%{!?dist:1}
%define rh_dist %(if [ -f /etc/redhat-release ];then cat /etc/redhat-release|sed "s/[^0-9.]*//"|cut -f1 -d.;fi)
%if 0%{?rh_dist}
%define dist .rhel%{rh_dist}
%else
%define dist .unknown
%endif
%endif

Summary: Qt4 module for Qore
Name: qore-qt4-module
Version: 0.1.0
Release: 1%{dist}
License: LGPL
Group: Development/Languages
URL: http://qore.org
Source: http://prdownloads.sourceforge.net/qore/%{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: gcc-c++
BuildRequires: qore-devel
BuildRequires: qore
BuildRequires: cmake
# qt4 4.5 minimal is required due the smoke generator code
%if 0%{?suse_version}
BuildRequires: libqt4-devel >= 4.5.0
%else
BuildRequires: qt4-devel >= 4.5.0
%endif

%description
Qt4 module for the Qore Programming Language. It provides
huge set of GUI widgets and frameworks to create rich user
interfaces.


%prep
%setup -q
cmake -DCMAKE_INSTALL_PREFIX=/usr .

%build
find test -name *.q|xargs chmod 644
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{module_dir}
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/qore-qt4-module
make install VERBOSE=1 DESTDIR=$RPM_BUILD_ROOT
rm $RPM_BUILD_ROOT/%{_libdir}/libqoresmokeqt.so

%post
ldconfig %{_libdir}

%postun
ldconfig %{_libdir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{module_dir}
%doc COPYING README test/* docs/*.html docs/*.css
%{_prefix}/bin/uic-qore
%{_libdir}/libqoresmokeqt.*


%changelog
* Thu Apr 15 2010 Petr Vanek <petr.vanek@qoretechnologies.com>
- initial package for Version 0.1.0
