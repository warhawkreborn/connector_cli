#include "server.h"


std::mutex Server::s_mutex;


Server::Server( warhawk::net::udp_server &udpServer_ )
  : m_server( udpServer_ )
  , m_MessageHandlers( )
{
}

Server::~Server( )
{
}


void Server::run( )
{
  struct sockaddr_in client;
  std::vector< uint8_t > data;

  while ( m_server.receive( client, data ) )
  {
    for ( auto itr : m_MessageHandlers )
    {
      MessageHandler *messageHandler = itr.second;

      messageHandler->OnReceivePacket( client, data );
    }
  }
}

void Server::send( struct sockaddr_in &clientaddr_, const std::vector< uint8_t > &data_ )
{
  m_server.send( clientaddr_, data_ );
}


bool Server::receive( struct sockaddr_in &clientaddr_, std::vector< uint8_t > &data_ )
{
  return m_server.receive( clientaddr_, data_ );
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
  return s_mutex;
}


void Server::Register( MessageHandler *handler_ )
{
  m_MessageHandlers[ handler_ ] = handler_;
}


void Server::Unregister( MessageHandler *handler_ )
{
  MessageHandlers::iterator itr = m_MessageHandlers.find( handler_ );

  if ( itr != m_MessageHandlers.end( ) )
  {
    m_MessageHandlers.erase( itr );
  }
}
