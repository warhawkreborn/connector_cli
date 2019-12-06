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

std::vector< uint8_t > Server::hex2bin( const std::string &str_ )
{
  if ( str_.size() % 2 )
  {
    throw std::runtime_error( "invalid hex string" );
  }

  std::vector< uint8_t > res;
  res.resize( str_.size() / 2 );

  for ( size_t i = 0; i < res.size(); i++ )
  {
    auto c = str_[ i * 2 ];

    if ( c >= 'A' && c <= 'F' )
    {
      res[ i ] = ( c - 'A' + 10 ) << 4;
    }
    else if ( c >= 'a' && c <= 'f' )
    {
      res[ i ] = ( c - 'a' + 10 ) << 4;
    }
    else if ( c >= '0' && c <= '9' )
    {
      res[ i ] = ( c - '0' ) << 4;
    }
    else
    {
      throw std::runtime_error( "invalid hex" );
    }

    c = str_[ i * 2 + 1 ];

    if ( c >= 'A' && c <= 'F' )
    {
      res[ i ] |= ( c - 'A' + 10 );
    }
    else if ( c >= 'a' && c <= 'f' )
    {
      res[ i ] |= ( c - 'a' + 10 );
    }
    else if ( c >= '0' && c <= '9' )
    {
      res[ i ] |= ( c - '0' );
    }
    else
    {
      throw std::runtime_error( "invalid hex" );
    }
  }

  return res;
}
