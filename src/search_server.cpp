#include <thread>

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
  struct sockaddr_in clientAddr;
  memset( (char *) &clientAddr, 0, sizeof( clientAddr ) );
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = htonl( 0xffffffff ); // 255.255.255.255 broadcast address.
  clientAddr.sin_port = htons( (unsigned short) WARHAWK_UDP_PORT );

  const std::vector< uint8_t > discoveryPacketData = m_server->hex2bin( m_DiscoveryPacket );

  while ( true )
  {
    std::cout << "SearchServer: Searching for new servers to publish." << std::endl;

    {
      // TODO: Do anything that needs to be done here for receiving packets while the mutex is locked.
    }

#ifdef FUTURE // - We need to be able to tell whether the packet came from us or not to prevent recursion
    const bool broadcast = true;
    m_server->send( clientAddr, discoveryPacketData, broadcast ); 
#endif

    std::this_thread::sleep_for( std::chrono::seconds( 30 ) );
  }
}


void SearchServer::OnReceivePacket( struct sockaddr_in client_, std::vector< uint8_t > data_ )
{
  std::cout << "SearchServer: Received packet." << std::endl;
}
