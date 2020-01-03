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

#include <functional>
#include <memory>
#include <vector>

#include "addr_info.h"
#include "ip_address.h"

struct addrinfo;

typedef struct IpData
{
  IpData( ) { }

  std::string m_InterfaceName;
  IpAddress   m_Address;
} IpData;


class Network
{
  public:

    //////////////////////////////
    // DECLARATIONS
    //////////////////////////////

    using IpAddresses_t = std::vector< IpData >;

    //////////////////////////////
    // METHODS
    //////////////////////////////

    Network( );
    ~Network( );

    const IpAddresses_t &GetMyIpAddresses( );

    bool OnAddressList( const IpAddresses_t &, const sockaddr_storage &address ) const; // True if this is one of my addresses.

    bool OnLocalNetwork( const std::string &ip ); // True if this ip is on one of the local networks.

    static std::vector< std::string > ResolveIpAddress( const std::string &hostname );

    void ForEachAddress( std::function< bool ( const IpData & ) > );

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

    //
    // Declarations
    //

    using IpAddrArray = std::array< unsigned char, sizeof( struct in6_addr ) >;

    //////////////////////////////
    // METHODS
    //////////////////////////////

    void _Init( );

    void AddAddress( IpAddresses_t &, const std::string &interfaceName, const char *address, const int prefixLen );

    int GetPrefixLen( const std::string &netmask ); // Return the prefix length of the nextmask;

    bool OnSameNetwork( const IpAddress &ipWithPrefix, const std::string &ip2 );

    void ConvertToInteger( const std::string &ip, IpAddrArray &out );

    bool Ipv4Addr( const std::string &ip );
    bool Ipv6Addr( const std::string &ip );

    // Return interface information from OS.
    std::string GetFirstInterface( );
    std::string GetNextInterface(  );

    //////////////////////////////
    // DATA
    //////////////////////////////

    IpAddresses_t m_MyIpAddresses; // List of my own IP addresses.
};
