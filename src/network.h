#pragma once

//
// The Network class provides various helper routines needed to do network processing,
// including keeping track of the list of IP addresses that are configured on this
// network host.
// For instance, this allows a caller to determine whether a received packet came from
// from itself or not, to prevent recursive loops on packet processing.
//

#if defined( __linux__ ) || defined( __APPLE__ )
// These return codes are Windows-specific so define them for Linux
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SOCKET int
#define DWORD uint32

#define closesocket( x ) close( x )

#else

#pragma warning( disable : 4786 )
#include <iphlpapi.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#endif

#include <memory>
#include <vector>

#include "addr_info.h"

struct addrinfo;

class Network
{
  public:

    //////////////////////////////
    // DECLARATIONS
    //////////////////////////////

    typedef std::vector< AddrInfo > IpAddresses_t;

    //////////////////////////////
    // METHODS
    //////////////////////////////

    Network( );
    ~Network( );

    const IpAddresses_t &GetMyIpAddresses( );

    bool OnAddressList( const IpAddresses_t &, const sockaddr_storage &address ) const; // True if this is one of my addresses.

    bool ResolveAddress( const char *inputAddr, std::string &resultString );

    //////////////////////////////
    // DATA
    //////////////////////////////

  protected:

    //////////////////////////////
    // METHODS
    //////////////////////////////

    //////////////////////////////
    // DATA
    //////////////////////////////

  private:

    //////////////////////////////
    // METHODS
    //////////////////////////////

    void _Init( );

    void AddAddress( IpAddresses_t &, const char *     );
    void AddAddress( IpAddresses_t &, const AddrInfo & );

    // Return interface information from OS.
    std::string GetFirstInterface( );
    std::string GetNextInterface(  );

    //////////////////////////////
    // DATA
    //////////////////////////////

    AddrInfo m_RemoteAddr; // Should default to IPv4 Broadcast or IPv6 Multi-cast but could also be IPv4 / IPv6 unicast.
    IpAddresses_t m_MyIpAddresses; // List of my own IP addresses.
};
