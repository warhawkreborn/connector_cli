#include "addr_info.h"
#include "network.h"
#include "packet.h"
#include "packet_processor.h"


PacketProcessor::PacketProcessor( const std::string &name_, warhawk::net::UdpNetworkSocket &udpNetworkSocket_, Network &network_ )
  : m_Name( name_ )
  , m_UdpNetworkSocket( udpNetworkSocket_ )
  , m_Network( network_ )
  , m_mutex( )
  , m_MessageHandlers( )
  , m_Thread( [ & ] ( ) { run( ); } )
{
}

PacketProcessor::~PacketProcessor( )
{
  m_Done = true;

  // Broadcast zero-length packet to make sure this server shuts down.
  AddrInfo clientAddr;
  clientAddr.SetAddr( "255.255.255.255" );
  clientAddr.SetPort( m_UdpNetworkSocket.GetPort( ) );
  std::vector< uint8_t > packet;
  const bool broadcast = true;
  m_UdpNetworkSocket.send( *clientAddr.GetAiAddr( ), packet, broadcast );

  m_Thread.join();
}


void PacketProcessor::run( )
{
  std::cout << "Packet Processor: '" << m_Name << "' starting" << std::endl;

  struct sockaddr_storage client;
  std::vector< uint8_t > data;

  while ( !m_Done && m_UdpNetworkSocket.receive( client, data ) )
  {
    std::string fromIp = AddrInfo::SockAddrToAddress( &client );
    bool fromLocalNetwork = m_Network.OnLocalNetwork( fromIp );
    uint16_t port = AddrInfo::SockAddrToPort( &client );

    Packet packet( data, fromLocalNetwork );

    packet.SetClient( client );
    packet.SetIp( fromIp );
    packet.SetPort( port );

    if ( packet.GetData( ).size( ) > 0 )
    {

      for ( auto itr : m_MessageHandlers )
      {
        MessageHandler *messageHandler = itr.first;
        int handlerMask                = itr.second;

        Packet::TYPE packetType = packet.GetType( );
        int result = (int) packetType & handlerMask;

        if ( result != 0 )
        { 
          messageHandler->OnReceivePacket( packet );
        }
      }
    }
  }

  std::cout << "Packet Processor: '" << m_Name << "' stopping." << std::endl;
}

void PacketProcessor::send( const sockaddr_storage &clientaddr_, const std::vector< uint8_t > &data_, bool broadcast_ )
{
  m_UdpNetworkSocket.send( clientaddr_, data_, broadcast_ );
}


bool PacketProcessor::receive( sockaddr_storage &clientaddr_, std::vector< uint8_t > &data_ )
{
  return m_UdpNetworkSocket.receive( clientaddr_, data_ );
}


warhawk::net::UdpNetworkSocket &PacketProcessor::GetServer()
{
  return m_UdpNetworkSocket;
}


void PacketProcessor::Register( MessageHandler *handler_, int messageMask_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );
  m_MessageHandlers[ handler_ ] = messageMask_;
}


void PacketProcessor::Unregister( MessageHandler *handler_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );
  MessageHandlers::iterator itr = m_MessageHandlers.find( handler_ );

  if ( itr != m_MessageHandlers.end( ) )
  {
    m_MessageHandlers.erase( itr );
  }
}

std::vector< uint8_t > PacketProcessor::hex2bin( const std::string &str_ )
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
