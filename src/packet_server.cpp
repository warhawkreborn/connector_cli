#include "addr_info.h"
#include "packet_server.h"


PacketServer::PacketServer( warhawk::net::udp_server &udpServer_ )
  : m_server( udpServer_ )
  , m_mutex( )
  , m_MessageHandlers( )
  , m_Thread( [ & ] ( ) { run( ); } )
{
}

PacketServer::~PacketServer()
{
  m_Done = true;

  // Broadcast zero-length packet to make sure this server shuts down
  AddrInfo clientAddr;
  clientAddr.SetAddr( "255.255.255.255" );
  clientAddr.PortToSockAddr( m_server.GetPort(), (sockaddr *) clientAddr.GetAiAddr() );
  std::vector< uint8_t > packet;
  const bool broadcast = true;
  m_server.send( *clientAddr.GetAiAddr(), packet, broadcast );

  m_Thread.join();
}


void PacketServer::run()
{
  std::cout << "Starting Packet Server..." << std::endl;

  struct sockaddr_storage client;
  std::vector< uint8_t > data;

  while ( !m_Done && m_server.receive( client, data ) )
  {
    if ( data.size( ) > 0 )
    {
      for ( auto itr : m_MessageHandlers )
      {
        MessageHandler *messageHandler = itr.second;

        messageHandler->OnReceivePacket( client, data );
      }
    }
  }

  std::cout << "Stopping Packet Server." << std::endl;
}

void PacketServer::send( const sockaddr_storage &clientaddr_, const std::vector< uint8_t > &data_, bool broadcast_ )
{
  m_server.send( clientaddr_, data_, broadcast_ );
}


bool PacketServer::receive( sockaddr_storage &clientaddr_, std::vector< uint8_t > &data_ )
{
  return m_server.receive( clientaddr_, data_ );
}


bool PacketServer::valid_packet( const std::vector< uint8_t > &data_ )
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


warhawk::net::udp_server &PacketServer::GetServer()
{
  return m_server;
}


void PacketServer::Register( MessageHandler *handler_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );
  m_MessageHandlers[ handler_ ] = handler_;
}


void PacketServer::Unregister( MessageHandler *handler_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );
  MessageHandlers::iterator itr = m_MessageHandlers.find( handler_ );

  if ( itr != m_MessageHandlers.end( ) )
  {
    m_MessageHandlers.erase( itr );
  }
}

std::vector< uint8_t > PacketServer::hex2bin( const std::string &str_ )
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
