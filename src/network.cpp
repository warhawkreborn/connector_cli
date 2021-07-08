#ifdef WIN32
#pragma warning ( disable : 4786 )
#include <WS2tcpip.h>
#include <Windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#endif // WIN32

#include <array>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <sstream>

#ifndef WIN32
#include <netdb.h>
#include <unistd.h>
#include <ifaddrs.h>
#endif

#include "network.h"


// Declare interface table data for finding out IP addresses (and other information 
// about the network interfaces.
static unsigned long mib_table_max_count = 0;
static unsigned long mib_index = 0;

#ifdef WIN32


#include <Iphlpapi.h>

static MIB_IFTABLE *mib_table = NULL;

#endif


//////////////////////////////////////////////////////////////////////
//  CONSTRUCTOR
//////////////////////////////////////////////////////////////////////
Network::Network( )
  : m_MyIpAddresses()
{
#ifdef OLD
  if ( StartWinsock( ) )
  {
    CreateServerSockets( recvAddr_, port_ );
  }
#endif

  _Init( );
}


//////////////////////////////////////////////////////////////////////
// DESTRUCTOR
//////////////////////////////////////////////////////////////////////
Network::~Network( )
{
#ifdef WIN32
  WSACleanup( );
#endif
}


#ifdef OLD
//////////////////////////////////////////////////////////////////////
//
//  StartWinsock( )
//
//////////////////////////////////////////////////////////////////////
bool Network::StartWinsock( )
{
#ifdef WIN32
  WORD    versionRequested;
  WSADATA wsaData;
 
  versionRequested = MAKEWORD( 2, 0 );
 
  int errorVal = WSAStartup( versionRequested, &wsaData );

  if ( errorVal == 0 )
  {
    //
    // Confirm that the WinSock DLL supports 2.0.
    // Note that if the DLL supports versions greater
    // than 2.0 in addition to 2.0, it will still return
    // 2.0 in wVersion since that is the version we     
    // requested.
    //
    if ( LOBYTE( wsaData.wVersion ) == 2 && HIBYTE( wsaData.wVersion ) == 0 )
    {
      // The WinSock DLL is acceptable.
      return( true );
    }
 
    WSACleanup( );
  }

  // TODO:  generate error message (WSAGetLastError())
 
  // Tell the user that we couldn't find a usable WinSock DLL.
  return( false );
#else
  return( true );
#endif
}
#endif

//////////////////////////////////////////////////////////////////////
//
//  _Init( )
//
//////////////////////////////////////////////////////////////////////
void Network::_Init( )
{
  AddAddress( m_MyIpAddresses, "127.0.0.1", 8 );

  AddAddress( m_MyIpAddresses, "::1", 128 );

#if defined( __linux__ ) || defined( __APPLE__ )
  struct ifaddrs *addrs = NULL;
  struct ifaddrs *tmp = NULL;

  if ( getifaddrs( &addrs ) != 0 )
  {
    if ( addrs != NULL )
    {
      freeifaddrs( addrs );
    }
    return; // Error.
  }

  for ( tmp = addrs; tmp; tmp = tmp->ifa_next )
  {
    char abuf[ 256 ];
    char nbuf[ 256 ];
    memset( &abuf[ 0 ], 0, sizeof(abuf) );
    memset( &nbuf[ 0 ], 0, sizeof(nbuf) );
    sockaddr *sa = tmp->ifa_addr;
    sockaddr *na = tmp->ifa_netmask;
    if ( sa != NULL )
    {
      switch ( sa->sa_family )
      {
        case AF_INET:
          inet_ntop( sa->sa_family, &((struct sockaddr_in *) sa)->sin_addr,
            abuf, sizeof(abuf) );
          inet_ntop( sa->sa_family, &((struct sockaddr_in *) na)->sin_addr,
            nbuf, sizeof(nbuf) );
          break;

        case AF_INET6:
          inet_ntop( sa->sa_family, &((struct sockaddr_in6 *) sa)->sin6_addr,
            abuf, sizeof(abuf) );
          inet_ntop( sa->sa_family, &((struct sockaddr_in6 *) na)->sin6_addr,
            nbuf, sizeof(nbuf) );
          break;

        default:
          break;
      }

      int prefixLen = 0;

      if ( nbuf[ 0 ] )
      {
        prefixLen = GetPrefixLen( nbuf );
      }

      if ( abuf[ 0 ] && nbuf[ 0 ] )
      {
        std::string address = abuf;
        // Delete '%' character and anything following if found.
        size_t idx = address.find( "%" );
        if ( idx != std::string::npos )
        {
          address = address.substr( 0, idx );
        }
        AddAddress( m_MyIpAddresses, address.c_str( ), prefixLen );
      }
    }
  }

  if ( addrs != NULL )
  {
    freeifaddrs( addrs );
  }

#endif

#ifdef WIN32
  PIP_ADAPTER_ADDRESSES pAddresses;
  pAddresses = (IP_ADAPTER_ADDRESSES*) malloc(sizeof(IP_ADAPTER_ADDRESSES));
  ULONG outBufLen = 0;
  DWORD dwRetVal = 0;

  // Make an initial call to GetAdaptersAddresses to get the 
  // size needed into the outBufLen variable
  if ( GetAdaptersAddresses( AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen ) == ERROR_BUFFER_OVERFLOW )
  {
    free(pAddresses);
    pAddresses = (IP_ADAPTER_ADDRESSES*) malloc( outBufLen );
  }

  // Make a second call to GetAdapters Addresses to get the
  // actual data we want
  if ( ( dwRetVal = GetAdaptersAddresses( AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen ) ) == NO_ERROR )
  {
    // If successful, output some information from the data we received
    while ( pAddresses )
    {
      for ( IP_ADAPTER_UNICAST_ADDRESS *ptr = pAddresses->FirstUnicastAddress;
            ptr != NULL;
            ptr = ptr->Next )
      {
        char buf[ 256 ];
        memset( &buf[0], 0, sizeof(buf) );
        getnameinfo( ptr->Address.lpSockaddr, ptr->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST );
        AddAddress( m_MyIpAddresses, buf, ptr->OnLinkPrefixLength );
      }

      pAddresses = pAddresses->Next;
    }
  }

#endif // WIN32

#if 0
  // Test - Convert back and make sure we got what we thought we got.
  for ( IpAddresses_t::iterator itr = m_MyIpAddresses.begin();
        itr != m_MyIpAddresses.end(); ++itr )
  {
    IpAddress *ptr = &(*itr);
    std::cout << "My Address: " << ptr->GetAddress( ) << "/" << ptr->GetPrefixLength( ) << std::endl;;
  }
#endif
}


//////////////////////////////////////////////////////////////////////
//
//  OnAddressList( )
//
//////////////////////////////////////////////////////////////////////
bool Network::OnAddressList( const IpAddresses_t &addrList_,
                             const sockaddr_storage &address_ ) const
{
  std::string incomingAddr = AddrInfo::SockAddrToAddress( &address_ ); 
  for ( IpAddresses_t::const_iterator itr = addrList_.begin( );
        itr != addrList_.end( ); ++itr )
  {
    const IpAddress *ptr = &( *itr );
    std::string address = ptr->GetAddress( );
    if ( incomingAddr == address )
    {
      return true; // Yes, this is one of my addresses.
    }
  }
  
  return false; // Nope, not one of my addresses.
}


//////////////////////////////////////////////////////////////////////
//
//  GetFirstInterface( )
//
//////////////////////////////////////////////////////////////////////
std::string Network::GetFirstInterface( )
{
  mib_index = 0; // Reset index to 0.

#ifdef WIN32
  if ( mib_table == 0 )
  {
    if ( GetNumberOfInterfaces( &mib_table_max_count ) != NO_ERROR ) // Find out required size
    {
      return "";
    }

    unsigned long mib_size = sizeof(DWORD) + ( mib_table_max_count * sizeof(MIB_IFROW) ) + 8;
    mib_table = (MIB_IFTABLE *) new char [ mib_size ];

    if ( GetIfTable( mib_table, &mib_size, true ) != NO_ERROR )
    {
      delete [] mib_table;
      mib_table_max_count = 0;
      return "";
    }
  }
#endif // WIN32

  return GetNextInterface( );
}


//////////////////////////////////////////////////////////////////////
//
//  GetNextInterface( )
//
//////////////////////////////////////////////////////////////////////
std::string Network::GetNextInterface( )
{
  char addrBuf[32];
  std::string addrString;

  if ( mib_index >= mib_table_max_count )
  {
    return "";
  }

  addrBuf[0] = '\0';

#ifdef WIN32

  MIB_IFROW * mibPtr = &mib_table->table[ mib_index++ ];

  if ( mibPtr->dwPhysAddrLen <= 0 )
  {
    return GetNextInterface();
  }

  sprintf( addrBuf, "%02x:%02x:%02x:%02x:%02x:%02x", mibPtr->bPhysAddr[0],
    mibPtr->bPhysAddr[1], mibPtr->bPhysAddr[2], mibPtr->bPhysAddr[3],
    mibPtr->bPhysAddr[4], mibPtr->bPhysAddr[5] );
#endif // WIN32
 
  addrString = addrBuf;
  return addrString;
}


void Network::AddAddress( IpAddresses_t &addrList_, const char *addr_, int prefixLen_ )
{
  IpAddress addr( addr_, prefixLen_ );
  addrList_.push_back( addr );
}


const Network::IpAddresses_t &Network::GetMyIpAddresses( )
{
  return m_MyIpAddresses;
}


int Network::GetPrefixLen( const std::string &netmask_ )
{
  int prefixLen = 0;

  if ( Ipv4Addr( netmask_) )
  {
    std::string netmask = netmask_;
    while ( netmask.length( ) > 0 )
    {
      size_t pos = netmask.find( "." );
      std::string num;
      if ( pos != std::string::npos )
      {
        num = netmask.substr( 0, pos );
        netmask = netmask.substr( pos + 1 );
      }
      else
      {
        num = netmask;
        netmask = "";
      }

      int bits = atoi( num.c_str( ) );
      // Count IPv4 bits.
      while ( bits != 0 )
      {
        if ( bits & 1 )
        {
          prefixLen++;
        }

        bits >>= 1;
      }
    }
  }
  else if ( Ipv6Addr( netmask_ ) )
  {
    // Count IPv6 bits.
    for ( unsigned char c : netmask_ )
    {
      int num = 0;

      if ( c >= '0' && c <= '9' )
      {
        num = c - '0';
      }
      else if ( c >= 'a' && c <= 'f' )
      {
        num = c - 'a' + 10;
      }
      else if ( c >= 'A' && c <= 'F' )
      {
        num = c - 'A' + 10;
      }
      else
      {
        continue;
      }

      while ( num != 0 )
      {
        if ( num & 1 )
        {
          prefixLen++;
        }

        num >>= 1;
      }
    }
  }

  return prefixLen;
}


bool Network::OnLocalNetwork( const std::string &ip_ )
{
  for ( const auto &entry : m_MyIpAddresses )
  {
    if ( OnSameNetwork( entry, ip_ ) )
    {
      return true;
    }
  }

  return false;
}


bool Network::OnSameNetwork( const IpAddress &ipWithPrefix_, const std::string &ip_ )
{
  IpAddrArray addrWithPrefix;
  IpAddrArray ip;
  std::string maskIp;
  IpAddrArray mask;

  if ( Ipv4Addr( ipWithPrefix_.GetAddress( ) ) && Ipv4Addr( ip_ ) )
  {
    maskIp = "255.255.255.255";
  }
  else if ( Ipv6Addr( ipWithPrefix_.GetAddress( ) ) && Ipv6Addr( ip_ ) )
  {
    maskIp = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";
  }
  else
  {
    return false; // Not on the same network because they are not the same type of address.
  }

  ConvertToInteger( ipWithPrefix_.GetAddress( ), addrWithPrefix );
  ConvertToInteger( ip_,                         ip             );
  ConvertToInteger( maskIp,                      mask           );

  int maskLen = ipWithPrefix_.GetPrefixLength( );
  int maskBytes = maskLen / 8 + ( maskLen % 8 == 0 ? 0 : 1 );

  for ( int i = 0; i < maskBytes; ++i, maskLen -= 8 )
  {
    unsigned char left  = addrWithPrefix[ i ];
    unsigned char right = ip[ i ];
    unsigned char mask = maskLen >= 8 ? 255 : ( 255 << maskLen ) ;
    bool sameNetwork = ( left & mask ) == right;

    if ( !sameNetwork )
    {
      return false;
    }
  }

  return true;
}


void Network::ConvertToInteger( const std::string &ip_, IpAddrArray &outBuf_ )
{
  memset( &outBuf_, 0, sizeof( outBuf_ ) );

  int domain = Ipv4Addr( ip_ ) ? AF_INET : Ipv6Addr( ip_ ) ? AF_INET6 : -1;

  if ( domain == -1 )
  {
    std::stringstream ss;
    ss << "FATAL ERROR: IP address '" << ip_ << "' not IPv4 or IPv6!";
    throw std::runtime_error( ss.str() );
  }

  int err = inet_pton( domain, ip_.c_str( ), &outBuf_ );

  if ( err <= 0 )
  {
    std::stringstream ss;
    ss << "FATAL ERROR: Can't convert IP '" << ip_ << "' to integer!" << std::endl;
    throw std::runtime_error( ss.str() );
  }
}


bool Network::Ipv4Addr( const std::string &ip_ )
{
  bool ipv4 = ip_.find( '.' ) != std::string::npos;
  return ipv4;
}


bool Network::Ipv6Addr( const std::string &ip_ )
{
  bool ipv6 = ip_.find( ':' ) != std::string::npos;
  return ipv6;
}


std::vector< std::string > Network::ResolveIpAddress( const std::string &hostname_ )
{
  std::vector< std::string > addressList;

  addrinfo *result = NULL;
  addrinfo hints;
  memset( &hints, 0, sizeof(hints) );

  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  hints.ai_family = AF_INET;

  hints.ai_flags =
#ifdef AI_ADDRCONFIG
    AI_ADDRCONFIG |
#endif
    0;

  int e = getaddrinfo( hostname_.c_str( ), "0", &hints, &result );

  if ( e != 0 )
  {
    if ( result != NULL )
    {
      freeaddrinfo( result );
    }

    return addressList; // Error.
  }

  for ( addrinfo *itr = result; itr != NULL; itr = itr->ai_next )
  {
    AddrInfo addrInfo( *itr );
    addressList.push_back( addrInfo.GetAddr( ) );
  }

  if ( result != NULL )
  {
    freeaddrinfo( result );
  }

  return addressList;
}


void Network::ForEachAddress( std::function< bool ( const IpAddress & ) > func_ )
{
  for ( const auto &ipAddr : m_MyIpAddresses )
  {
    bool continueOn = func_( ipAddr );

    if ( !continueOn )
    {
      break;
    }
  }
}
 
