# connector_unix
Connector to allow discovery of PlayStation 3 WarHawk servers and hosting local servers.

Works on Linux ( CentOS 7, Debian, Raspberry Pi ), MacOS, and Windows.

### Build Prerequisites

You need to install a couple of dependencies:

Linux ( Debian, RaspberryPi ):

```sudo apt install make build-essential libcurl4-openssl-dev libboost-system-dev libssl-dev```

### Compiling / Building

Linux ( CentOS 7):

- cd packaging/centos-7
- make

Linux (Debian, RaspberryPi ):

- mkdir build
- cd build
- cmake ..
- cmake
- cpack

MacOS:

- cd packaging/macos
- make

Windows:

- mkdir build
- cd build
- cmake ..
- cmake
- cpack

It should successfully compile and generate an installer.

### Installing

Linux ( CentOS 7 ):

Install the generated RPM file.
- systemctl enable warhawk
- systemctl start warhawk

Linux ( Debian, RaspberryPi ):

Install the generated DEB file.

It should automatically start the service.

MacOS:

Install the PKG file.
Then execute the following command
- /usr/local/bin/warhawkreborn

Windows:

Install the EXE installer.

Execute the following program.
- \Program Files\WarHawkReborn 1.1.0\warhawkreborn.exe


### Usage

Go to your PS3, enter local games mode and search for games.

## Web Server built in to warhawkreborn program

http://localhost:8080/

(Substitute 'localhost' with another name if you are accessing it remotely)

### Status

https://warhawk.thalhammer.it/
