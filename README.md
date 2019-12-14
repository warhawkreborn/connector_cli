# connector_unix
Connector to allow discovery of servers and hosting local servers.

Works on Linux ( CentOS 7, Debian, Raspberry Pi ), MacOS, and Windows.

### Build Prerequisites

You need to install a couple of dependencies:

Linux ( Debian, RaspberryPi ):

```sudo apt install make build-essential libcurl4-openssl-dev libboost-system-dev libssl-dev```

### Compiling / Building

Linux ( CentOS 7): cd packaging/centos-7; make

Linux (Debian, RaspberryPi ): mkdir build; cd build; cmake ..; cmake; cpack

MacOS: cd packaging/macos; make

Windows: mkdir build; cd build; cmake ..; cmake; cpack

It should successfully compile and generate an installer.

### Installing

Linux ( CentOS 7 ):

Install the generated RPM file.
systemctl enable warhawk
systemctl start warhawk

Linux ( Debian, RaspberryPi ):

Install the generated DEB file.

It should automatically start the service.

MacOS:

Install the PKG file.
Execute the /usr/local/bin/warhawk/reborn command from a terminal.

Windows:

Install the EXE installer.

Execute the /Program Files/WarHawkReborn 1.1.0\warhawkreborn.exe program.


### Usage

Go to your PS3, enter local games mode and search for games.
