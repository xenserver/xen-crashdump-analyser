Name: xen-crashdump-analyser
Summary: Xen crashdump analyser
Version: 2.4.1
Release: 1
License: GPL
Group: Applications/System
Source: xen-crashdump-analyser.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
#%define debug_package %{nil}
%description
A tool for analysing Xen crashes. See http://xenbits.xen.org/people/andrewcoop/
for detailed usage information.

%prep
%setup -q -n %{name}

%build
make

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p -m755 $RPM_BUILD_ROOT%{_libdir}/xen/bin/

install -m755 xen-crashdump-analyser $RPM_BUILD_ROOT%{_libdir}/xen/bin/xen-crashdump-analyser

%clean
rm -rf $RPM_BUILD_ROOT

%post

%files
%defattr(-,root,root,-)
%{_libdir}/xen/bin/xen-crashdump-analyser

%changelog
