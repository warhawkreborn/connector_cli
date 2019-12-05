#include "server.h"


std::mutex Server::s_mtx;


Server::Server( warhawk::net::udp_server &udpServer_ ) : m_server( udpServer_ )
{
}

Server::~Server( )
{
}


  bool Server::valid_packet( const std::vector< uint8_t > &data_ )
{
  if ( data_.size() < 4 )
  {
    return false;
  }

  uint16_t len = data_[ 3 ];
  len = ( len << 8 ) | data_[ 2 ];

  if ( data_.size() - 4 != len )
  {
    return false;
  }

  return true;
}


warhawk::net::udp_server &Server::GetServer( )
{
  return m_server;
}


std::mutex &Server::GetMutex( )
{
  return s_mtx;
}
