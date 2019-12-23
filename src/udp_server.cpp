#ifdef WIN32
#include <WS2tcpip.h>
#include <Windows.h>
#include <winsock2.h>
#endif

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <sstream>
#include <sys/types.h>

#ifndef WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "udp_server.h"
#include "network.h"


namespace warhawk
{

namespace net
{

udp_server::udp_server( uint16_t port_ )
  : m_fd( 0 )
  , m_port( port_ )
  , m_Network( nullptr )
{
  m_fd = socket( AF_INET, SOCK_DGRAM, 0 );

  if ( m_fd < 0 )
  {
    std::stringstream ss;
    ss << "ERROR opening socket: " << errno << ".  Check to see if port " << m_port << " is already in use.";
    throw std::runtime_error( ss.str( ).c_str( ) );
  }

  int optval = 1;
  setsockopt( m_fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof( optval ) );

  struct sockaddr_in serveraddr;
  memset( (char *) &serveraddr, 0, sizeof( serveraddr ) );
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );
  serveraddr.sin_port = htons( (unsigned short) m_port );

  if ( bind( m_fd, (struct sockaddr *) &serveraddr, sizeof( serveraddr ) ) < 0 )
  {
    throw std::runtime_error( "ERROR on binding" );
  }

  // Set up for determining if I'm receiving packets from myself.
  m_Network = new Network;
}


udp_server::~udp_server( )
{
  delete m_Network;
}


void udp_server::send( const sockaddr_storage &clientaddr_, const std::vector< uint8_t > &data_, const bool broadcast_ )
{
  int optval = broadcast_ ? 1 : 0;
  int err = setsockopt( m_fd, SOL_SOCKET, SO_BROADCAST, (const char *) &optval, sizeof( optval ) );

  int n = sendto(
    m_fd, (const char *) data_.data(), (size_t) data_.size(), 0, (sockaddr *) &clientaddr_, sizeof( sockaddr ) );

  if ( n != (int) data_.size() )
  {
    int err = 0;
#ifdef WIN32

    if ( n == SOCKET_ERROR )
    {
      err = WSAGetLastError( );
    }
#else
    if ( n == SOCKET_ERROR )
    {
      err = errno;
    }
#endif

    std::stringstream ss;
    ss << "Failed to send data, n = " << n << ", error = " << err; 
    throw std::runtime_error( ss.str( ).c_str( ) );
  }
}


bool udp_server::receive( sockaddr_storage &clientaddr_, std::vector< uint8_t > &data_ )
{
  data_.resize( 16 * 1024 ); // TODO: Detect MTU
  socklen_t clientlen = sizeof( clientaddr_ );

  int n = recvfrom( m_fd, (char *) data_.data( ), (int) data_.size( ), 0, (sockaddr *) &clientaddr_, &clientlen );

  if ( n < 0 )
  {
    return false;
  }

  // Did this come from me?
  if ( m_Network->OnAddressList( m_Network->GetMyIpAddresses( ), clientaddr_ ) )
  {
    // If yes, then return true but with a zero data length packet.
    n = 0;
  }

  data_.resize( n );

  return true;
}


std::array< uint8_t, 4 > udp_server::StringToIp( const std::string &host_ )
{
  auto host_entry = gethostbyname( host_.c_str() );

  if ( host_entry == nullptr )
  {
    throw std::runtime_error( "Failed to get hostname" );
  }

  auto addr = (struct in_addr *) host_entry->h_addr_list[ 0 ];
  auto data = (const uint8_t *) &addr->s_addr;
  return { data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] };
}


uint16_t udp_server::GetPort( ) const
{
  return m_port;
}


std::string udp_server::IpToString( const std::array< uint8_t, 4 > &ip_ )
{
  return std::to_string( ip_[ 0 ] ) + "." + std::to_string( ip_[ 1 ] ) + "." +
         std::to_string( ip_[ 2 ] ) + "." + std::to_string( ip_[ 3 ] );
}

} // namespace net

} // namespace warhawk
