#include <assert.h>
#include <string.h>

#include "addr_info.h"

#ifdef WIN32
#define strdup _strdup
#endif


//////////////////////////////////////////////////////////////////////
//
//  AddrInfo( )
//
//////////////////////////////////////////////////////////////////////
AddrInfo::AddrInfo( )
{
}


AddrInfo::AddrInfo( const addrinfo &info_ )
{
  Set( info_ );
}


AddrInfo::~AddrInfo( )
{
}


void AddrInfo::Set( const addrinfo &info_ )
{
  m_ai_flags      = info_.ai_flags;
  m_ai_family     = info_.ai_family;
  m_ai_socktype   = info_.ai_socktype;
  m_ai_protocol   = info_.ai_protocol;
  m_ai_addrlen    = (socklen_t) info_.ai_addrlen;
  m_ai_canonname  = info_.ai_canonname == NULL ? "" : info_.ai_canonname;
  memcpy( &m_ai_addr, info_.ai_addr, m_ai_addrlen );
}


void AddrInfo::Get( addrinfo &info_ ) const
{
  info_.ai_flags     = m_ai_flags; 
  info_.ai_family    = m_ai_family; 
  info_.ai_socktype  = m_ai_socktype;
  info_.ai_protocol  = m_ai_protocol;
  info_.ai_addrlen   = m_ai_addrlen;
  info_.ai_canonname = strdup( m_ai_canonname.c_str() );
  info_.ai_addr      = (sockaddr *) malloc( sizeof(m_ai_addr) );
  memcpy( info_.ai_addr, &m_ai_addr, m_ai_addrlen );
}


const sockaddr_storage *AddrInfo::GetAiAddr( ) const
{
  return &m_ai_addr;
}


socklen_t AddrInfo::GetAiAddrLen( ) const
{
  return m_ai_addrlen;
}


int AddrInfo::GetAiFamily( ) const
{
  return m_ai_family;
}


// get sockaddr, IPv4 or IPv6:
void *AddrInfo::GetInAddr( const struct sockaddr *sa_ )
{
  if ( sa_->sa_family == AF_INET )
  {
    return &( ( (struct sockaddr_in * ) sa_ )->sin_addr );
  }
  else if ( sa_->sa_family == AF_INET6 )
  {
    return &( ( ( struct sockaddr_in6 * ) sa_ )->sin6_addr );
  }

  return NULL;
}


std::string AddrInfo::SockAddrToAddress( const struct sockaddr *sa_ )
{
  struct addrinfo p;
  p.ai_addr = (struct sockaddr *) sa_;
  p.ai_family = sa_->sa_family;

  char s[INET6_ADDRSTRLEN];
  inet_ntop( p.ai_family, GetInAddr( (struct sockaddr *) p.ai_addr ), s, sizeof( s ) );
  std::string stringAddress;
  stringAddress = s;
  return stringAddress;
}


uint16_t AddrInfo::SockAddrToPort( const struct sockaddr *sa_ )
{
  uint16_t port = 0;

  if ( sa_->sa_family == AF_INET )
  {
    port = ( ( ( struct sockaddr_in * ) sa_ )->sin_port );
  }
  else if ( sa_->sa_family == AF_INET6 )
  {
    port = ( ( ( struct sockaddr_in6 * ) sa_ )->sin6_port );
  }

  port = ntohs( port );

  return port;
}


void AddrInfo::PortToSockAddr( const uint16_t port_, struct sockaddr *sa_ )
{
  uint16_t port = htons( port_ );

  if ( sa_->sa_family == AF_INET )
  {
    ( ( ( struct sockaddr_in * ) sa_ )->sin_port ) = port;
  }
  else if ( sa_->sa_family == AF_INET6 )
  {
    ( ( ( struct sockaddr_in6 * ) sa_ )->sin6_port ) = port;
  }
}


std::string AddrInfo::GetAddr( ) const
{
  static char buf[ 256 ];
  memset( &buf[0], 0, sizeof(buf) );

  addrinfo ai;
  memset( &ai, 0, sizeof(ai) );

  Get( ai );

  int e = getnameinfo( ai.ai_addr,
                       (socklen_t) ai.ai_addrlen,
                       buf,
                       sizeof( buf ),
                       NULL,
                       0,
                       NI_NUMERICHOST );

  free( ai.ai_canonname );
  free( ai.ai_addr );

  assert( e == 0 );

  if ( e != 0 )
  {
    return NULL;
  }

  return &buf[0];
}


bool AddrInfo::SetAddr( const std::string &address_ )
{
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

  int e = getaddrinfo( address_.c_str( ), "0", &hints, &result );

  if ( e != 0 )
  {
    if ( result != NULL )
    {
      freeaddrinfo( result );
    }

    return false; // Error.
  }

  for ( addrinfo *itr = result; itr != NULL; itr = itr->ai_next )
  {
    Set( *itr );
  }

  if ( result != NULL )
  {
    freeaddrinfo( result );
  }

  return true; // OK.
}
