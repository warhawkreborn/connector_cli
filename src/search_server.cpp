#include <thread>

#include "addr_info.h"
#include "net.h"
#include "search_server.h"

SearchServer::SearchServer( Server *server_ )
  : m_mutex( )
  , m_entries( )
  , m_server( server_ )
{
  m_server->Register( this );
}


SearchServer::~SearchServer( )
{
  m_server->Unregister( this );
}


void SearchServer::run( )
{
  AddrInfo clientAddr;
  clientAddr.SetAddr( "255.255.255.255" );
  clientAddr.PortToSockAddr( m_server->GetServer( ).GetPort( ), (sockaddr *) clientAddr.GetAiAddr( ) );

  const std::vector< uint8_t > discoveryPacketData = m_server->hex2bin( m_DiscoveryPacket );

  while ( true )
  {
    std::cout << "SearchServer: Searching for new servers to publish." << std::endl;

    // Broadcast Server Discovery Packet
    const bool broadcast = true;
    m_server->send( *clientAddr.GetAiAddr( ), discoveryPacketData, broadcast ); 

    std::this_thread::sleep_for( std::chrono::seconds( 30 ) );
  }
}


void SearchServer::OnReceivePacket( struct sockaddr_storage client_, std::vector< uint8_t > data_ )
{
  std::cout << "SearchServer: Received packet." << std::endl;
}
