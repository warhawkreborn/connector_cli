#ifdef WIN32
#pragma warning ( disable : 4786 )
#include <WS2tcpip.h>
#include <Windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#endif // WIN32

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
  AddAddress( m_MyIpAddresses, "127.0.0.1" );

  AddAddress( m_MyIpAddresses, "::1" );

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
    char buf[ 256 ];
    memset( &buf[0], 0, sizeof(buf) );
    sockaddr *sa = tmp->ifa_addr;
    if ( sa != NULL )
    {
      switch ( sa->sa_family )
      {
        case AF_INET:
          inet_ntop( sa->sa_family, &((struct sockaddr_in *) sa)->sin_addr,
            buf, sizeof(buf) );
          break;

        case AF_INET6:
          inet_ntop( sa->sa_family, &((struct sockaddr_in6 *) sa)->sin6_addr,
            buf, sizeof(buf) );
          break;

        default:
          break;
      }

      if ( buf[0] )
      {
        addrinfo *ai = NULL;

        int e = getaddrinfo( &buf[0], NULL, NULL, &ai );
        if ( e != 0 )
        {
          if ( addrs != NULL )
          {
            freeifaddrs( addrs );
          }
          // FIXME - Add error checking.
          return; // Error.
        }

        if ( ai )
        {
          AddrInfo info( *ai );
          AddAddress( m_MyIpAddresses, info );
          freeaddrinfo( ai );
        }
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
  if ( GetAdaptersAddresses( AF_UNSPEC, 0, NULL, pAddresses, &outBufLen ) == ERROR_BUFFER_OVERFLOW )
  {
    free(pAddresses);
    pAddresses = (IP_ADAPTER_ADDRESSES*) malloc( outBufLen );
  }

  // Make a second call to GetAdapters Addresses to get the
  // actual data we want
  if ( ( dwRetVal = GetAdaptersAddresses( AF_UNSPEC, 0, NULL, pAddresses, &outBufLen ) ) == NO_ERROR )
  {
    // If successful, output some information from the data we received
    while ( pAddresses )
    {
      for ( IP_ADAPTER_UNICAST_ADDRESS *ptr = pAddresses->FirstUnicastAddress;
            ptr != NULL; ptr = ptr->Next )
      {
        char buf[ 256 ];
        memset( &buf[0], 0, sizeof(buf) );
        getnameinfo( ptr->Address.lpSockaddr, ptr->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST );

        addrinfo *ai = NULL;

        int e = getaddrinfo( &buf[0], NULL, NULL, &ai );
        if ( e != 0 )
        {
          // FIXME - Add error checking.
          return; // Error.
        }

        if ( ai )
        {
          AddrInfo info( *ai );
          AddAddress( m_MyIpAddresses, info );
          freeaddrinfo( ai );
        }
      }

      for ( IP_ADAPTER_ANYCAST_ADDRESS *ptr = pAddresses->FirstAnycastAddress;
            ptr != NULL; ptr = ptr->Next )
      {
        char buf[ 256 ];
        memset( &buf[0], 0, sizeof(buf) );
        getnameinfo( ptr->Address.lpSockaddr, ptr->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST );

        addrinfo *ai = NULL;

        int e = getaddrinfo( &buf[0], NULL, NULL, &ai );
        if ( e != 0 )
        {
          // FIXME - Add error checking.
          return; // Error.
        }

        if ( ai )
        {
          AddrInfo info( *ai );
          AddAddress( m_MyIpAddresses, info );
          freeaddrinfo( ai );
        }
      }

      pAddresses = pAddresses->Next;
    }
  }

#endif // WIN32

#if 1
  // Test - Convert back and make sure we got what we thought we got.
  for ( IpAddresses_t::iterator itr = m_MyIpAddresses.begin();
        itr != m_MyIpAddresses.end(); ++itr )
  {
    AddrInfo *ptr = &(*itr);
    std::cout << "My Address: " << ptr->GetAddr( ) << std::endl;;
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
  std::string incomingAddr = AddrInfo::SockAddrToAddress( (const sockaddr *) &address_ ); 
  for ( IpAddresses_t::const_iterator itr = addrList_.begin( );
        itr != addrList_.end( ); ++itr )
  {
    const AddrInfo *ptr = &( *itr );
    std::string address = ptr->GetAddr( );
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


void Network::AddAddress( IpAddresses_t &addrList_, const char *addr_ )
{
  addrinfo *ai = NULL;

  int e = getaddrinfo( addr_, NULL, NULL, &ai );
  if ( e != 0 )
  {
    // FIXME - Add error checking.
    return; // Error.
  }

  if ( ai )
  {
    AddrInfo info( *ai );

    AddAddress( addrList_, info );

    freeaddrinfo( ai );
  }
}


void Network::AddAddress( IpAddresses_t &addrList_, const AddrInfo &info_ )
{
  bool found = false;
  for ( IpAddresses_t::iterator itr = addrList_.begin( );
        itr != addrList_.end( ); ++itr )
  {
    if ( itr->GetAiAddrLen( ) == info_.GetAiAddrLen( ) &&
         memcmp( itr->GetAiAddr( ), info_.GetAiAddr( ), itr->GetAiAddrLen( ) ) == 0 )
    {
      found = true;
      break;
    } 
  }

  if ( !found )
  {
    addrList_.push_back( info_ );
  }
}
