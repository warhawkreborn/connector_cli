Name: warhawk-reborn
Version: 1.1
Release: @RELEASE@%{?dist}
Vendor: David Summers
URL: https://warhawk.thalhammer.it
License: GPL
Summary: WarHawk lives again on the Sony PlayStation 3
Source: warhawk-reborn-%{version}.tar.gz
Group: Utilities/System
Packager: david@summersoft.fay.ar.us
BuildRequires: cmake3
BuildRequires: devtoolset-8
ExcludeArch: noarch

%description
Play Internet WarHawk again on the Sony PlayStation 3.

This acts as a simple bridge between the PlayStation 3 and a host that contains a list of servers.

If you set your router port forwarding to forward port 10029 to you PlayStation 3 then you can also host WarHawk games.

%changelog
* Sat Dec 07 2019 David Summers <david@summersoft.fay-ar.us> @VERSION@-@RELEASE@-@BUILD@
- First version that can host WarHawk servers.

%prep
%setup -q

%build
# Build the software.
mkdir build
cd build

scl enable devtoolset-8 bash <<EOF
gcc --version
cmake3 ..
EOF

make

%install
cd build
/bin/rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin

# Install executable
cp src/warhawk-reborn $RPM_BUILD_ROOT/usr/bin

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README.md
/usr/bin/warhawk-reborn
