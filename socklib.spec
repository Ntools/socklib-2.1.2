Name:		socklib
Summary:	Socket library
Version:	2.1.2
Release:	6
Group:		Libraries
License:	BSD
Vendor:		Ntools Project
URL:		http://www.ntools.net/
Source:		http://www.ntools.net/pub/network/socklib/socklib-%{version}.tar.bz2
BuildRoot:	/var/tmp/%{name}-root

%package mingw32
Summary: development for i386-mingw(win32 gcc) library
Group: Development/Libraries
Requires: mingw32-gcc

%description
Usefull Network & UNIX Socket Library

%description mingw32
Usefull Network Socket Library for i386-mingw32.

%prep
%setup -q 

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib
mkdir -p $RPM_BUILD_ROOT%{_prefix}/include
mkdir -p $RPM_BUILD_ROOT%{_prefix}/share/doc/socklib-%{version}
mkdir -p $RPM_BUILD_ROOT%{_prefix}/local/i386-mingw32/include
mkdir -p $RPM_BUILD_ROOT%{_prefix}/local/i386-mingw32/lib
strip libnsock.so.%{version}
install -c -m 755 libnsock.so.%{version} $RPM_BUILD_ROOT%{_prefix}/lib
install -c -m 644 libnsock.a $RPM_BUILD_ROOT%{_prefix}/lib
ln -s libnsock.so.%{version} $RPM_BUILD_ROOT%{_prefix}/lib/libnsock.so.1
ln -s libnsock.so.1 $RPM_BUILD_ROOT%{_prefix}/lib/libnsock.so
install -c -m 644 nsock.h $RPM_BUILD_ROOT%{_prefix}/include
install -c -m 644 LICENSE $RPM_BUILD_ROOT%{_prefix}/share/doc/socklib-%{version}

install -c -m 644 nsock.h $RPM_BUILD_ROOT%{_prefix}/local/i386-mingw32/include
cp -a libnsockwin32.a $RPM_BUILD_ROOT%{_prefix}/local/i386-mingw32/lib/libnsock.a

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_prefix}/include/*
%{_prefix}/lib/*
%{_prefix}/share/doc/*

%files mingw32
%defattr(-,root,root)
%{_prefix}/local/i386-mingw32

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%changelog
* Thu Jun 26 2008 Nobby N Hirano <nobby@ntools.net>
- Fix tcp_IPadress io parameters.

* Thu Nov 8 2005 Nobby N Hirano <nobby@ntools.net>
- Fix 4 cross compiler environment

* Thu Oct 13 2005 Nobby N Hirano <nobby@ntools.net>
- Fix value int to unsigned for timeout functions

* Fri Oct 7 2005 Nobby N Hirano <nob@ntools.net>
- Add functions:
-   UDP timeout function
 
* Sun Apr 3 2005 Nobby N Hirano <nob@ntools.net>
- LICENSE change
- Add win32 sources

* Thu Mar 3 2005 Nobby N Hirano <nob@ntools.net>
- fix nsock.h

* Mon May 3 2004 Nobby N Hirano <nob@ntools.net>
- add functions:
- With timeout each functions, Read Write Printf.

* Sun Apr 18 2004 Nobby N Hirano <nob@ntools.net>
- removed handlers

* Wed Mar 17 2004 Nobby N Hirano <nob@ntools.net>
- Fix event handlers

* Fri Apr 11 2003 Nobby N Hirano <nob@ntools.net>
- event handlers add

* Sat Apr 6 2003 Nobby N Hirano <nob@ntools.net>
- Add WSockStatus for Writable check

* Wed Nov 13 2002 Nobby N Hirano <nobby@hiug.jp>
- Add Local communication (PF_UNIX)

* Sat Aug 17 2002 Nobby N Hirano <nobby@opensource.ne.jp>
- Fix act_rec client IP return value

* Sat May 4 2002 Nobby N Hirano <nobby@hiug.ne.jp>
- little Fix include file.
 
* Mon Feb 4 2002 Nobby N Hirano <nobby@hiug.ne.jp>
- Build !
