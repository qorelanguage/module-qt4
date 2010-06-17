%define module_api %(qore --latest-module-api 2>/dev/null)
%define module_dir %{_libdir}/qore-modules

%if 0%{?sles_version}

%define dist .sles%{?sles_version}

%else
%if 0%{?suse_version}

%if 0%{?suse_version} == 1130
%define dist .opensuse11_3
%endif

%if 0%{?suse_version} == 1120
%define dist .opensuse11_2
%endif

%if 0%{?suse_version} == 1110
%define dist .opensuse11_1
%endif

%if 0%{?suse_version} == 1100
%define dist .opensuse11
%endif

%if 0%{?suse_version} == 1030
%define dist .opensuse10_3
%endif

%if 0%{?suse_version} == 1020
%define dist .opensuse10_2
%endif

%if 0%{?suse_version} == 1010
%define dist .suse10_1
%endif

%if 0%{?suse_version} == 1000
%define dist .suse10
%endif

%if 0%{?suse_version} == 930
%define dist .suse9_3
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