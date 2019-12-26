Name: WarHawkReborn
Version: @VERSION@
Release: @RELEASE@%{?dist}
Vendor: David Summers
URL: https://warhawk.thalhammer.it
License: GPL
Summary: WarHawk lives again on the Sony PlayStation 3
Source: %{name}-%{version}.tar.gz
Group: Utilities/System
Packager: david@summersoft.fay.ar.us
BuildRequires: cmake3
BuildRequires: devtoolset-8
ExcludeArch: noarch

%description
Play Sony PlayStation3 WarHawk on the Internet again.

This acts as a simple bridge between the PlayStation 3 and a host that contains a list of servers.

If you set your router port forwarding to forward port 10029 to your PlayStation 3 then you can also *HOST* WarHawk games for others to join.

%changelog
* Thu Dec 26 2019 David Summers <david@summersoft.fay-ar.us> @VERSION@-@RELEASE@
- Added web server at (default) port 8080.

* Sat Dec 07 2019 David Summers <david@summersoft.fay-ar.us> 1.1-1c1251b
- First version that can host a local WarHawk server so that other Internet
  players can join.

%prep
%setup -q

%build
# Build the software.
mkdir build
cd build

scl enable devtoolset-8 bash <<EOF
gcc --version
cmake3 .. -DGIT_HASH=@RELEASE@
EOF

make

%install
/bin/rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin

# Install executable
cp build/src/warhawkreborn $RPM_BUILD_ROOT/usr/bin

# Install systemd service
mkdir -p $RPM_BUILD_ROOT/usr/lib/systemd/system
cp packages/centos-7/warhawk.service $RPM_BUILD_ROOT/usr/lib/systemd/system

# Install html pages
mkdir -p $RPM_BUILD_ROOT/usr/share/warhawkreborn
cp -r html $RPM_BUILD_ROOT/usr/share/warhawkreborn

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README.md
/usr/bin/warhawkreborn
/usr/lib/systemd/system/warhawk.service
/usr/share/warhawkreborn
