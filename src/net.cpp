#ifdef WIN32
#include <WS2tcpip.h>
#include <Windows.h>
#include <winsock2.h>
#endif

#include <cstdio>
#include <stdexcept>
#include <string.h>
#include <string>
#include <sys/types.h>

#ifndef WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "net.h"

namespace warhawk
{

namespace net
{

udp_server::udp_server( uint16_t port_ )
{
  m_fd = socket( AF_INET, SOCK_DGRAM, 0 );

  if ( m_fd < 0 )
  {
    throw std::runtime_error( "ERROR opening socket" );
  }

  int optval = 1;
  setsockopt( m_fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof( optval ) );

  struct sockaddr_in serveraddr;
  memset( (char *) &serveraddr, 0, sizeof( serveraddr ) );
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );
  serveraddr.sin_port = htons( (unsigned short) 10029 );

  if ( bind( m_fd, (struct sockaddr *) &serveraddr, sizeof( serveraddr ) ) < 0 )
  {
    throw std::runtime_error( "ERROR on binding" );
  }
}


udp_server::~udp_server( )
{
}


void udp_server::send( struct sockaddr_in &clientaddr_, const std::vector< uint8_t > &data_ )
{
  std::unique_lock< std::mutex > lck( m_mtx );

  int n =
    sendto( m_fd, (const char *) data_.data( ), (int) data_.size( ), 0, (struct sockaddr *) &clientaddr_, sizeof( clientaddr_ ) );

  if ( n != data_.size() )
  {
    throw std::runtime_error( "failed to send data" );
  }
}


bool udp_server::receive( struct sockaddr_in &clientaddr_, std::vector< uint8_t > &data_ )
{
  std::unique_lock< std::mutex > lck( m_mtx );

  data_.resize( 16 * 1024 ); // TODO: Detect MTU
  socklen_t clientlen = sizeof( clientaddr_ );

  int n = recvfrom( m_fd, (char *) data_.data( ), (int) data_.size( ), 0, (struct sockaddr *) &clientaddr_, &clientlen );

  if ( n < 0 )
  {
    return false;
  }

  data_.resize( n );

  return true;
}


std::array< uint8_t, 4 > udp_server::get_ip( const std::string &host_ )
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

/*std::string udp_server::ip_to_string(uint32_t ip, uint16_t port) {
    char *hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
        throw std::runtime_error("ERROR on inet_ntoa");
    struct hostent *hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr),
AF_INET); const char *hname = hostp ? hostp->h_name : ""; std::cout << "server received " << n << " bytes from " <<
hname << " (" << hostaddrp << ")" << std::endl;
}*/

} // namespace net

} // namespace warhawk
