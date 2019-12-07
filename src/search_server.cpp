#include <thread>

#include "addr_info.h"
#include "net.h"
#include "search_server.h"

SearchServer::SearchServer( Server *server_ )
  : m_mutex( )
  , m_entries( )
  , m_server( server_ )
  , m_CurrentState( STATE::STATE_WAITING )
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
    switch ( m_CurrentState )
    {
      case STATE::STATE_WAITING:
      {
        std::this_thread::sleep_for( std::chrono::seconds( 30 ) );

        std::cout << "SearchServer: Broadcasting - Searching for new servers to publish." << std::endl;

        // Broadcast Server Discovery Packet
        const bool broadcast = true;
        m_server->send( *clientAddr.GetAiAddr( ), discoveryPacketData, broadcast ); 

        m_CurrentState = STATE::STATE_COLLECTING;

        break;
      }

      case STATE::STATE_COLLECTING:

        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

        m_CurrentState = STATE::STATE_PROCESSING;

        break;

      case STATE::STATE_PROCESSING:

        std::cout << "SearchServer: Processing any collected packets." << std::endl;

        // Now do something with any packets collected.
        {
          std::unique_lock< std::mutex > lck( m_mutex );

          if ( m_PacketList.size( ) != 0 )
          {
            std::cout << "SearchServer: Processing " << m_PacketList.size( ) << " packets." << std::endl;
            for ( PacketList::iterator itr = m_PacketList.begin( ); itr != m_PacketList.end( ); )
            {
              PacketData &data = *itr;
              std::cout << "SearchServer: Local server found at IP " << data.m_address                << std::endl;
              std::cout << "SearchServer: Name = '"                  << data.m_data.GetName( ) << "'" << std::endl;
              std::cout << "SearchServer: MapName = "                << data.m_data.GetMapName( )     << std::endl;
              std::cout << "SearchServer: GameMode = "               << data.m_data.GetGameMode( )    << std::endl;
              itr = m_PacketList.erase( itr );
            }
          }
        }

        m_CurrentState = STATE::STATE_WAITING;

        break;
    }
  }
}


void SearchServer::OnReceivePacket( struct sockaddr_storage client_, std::vector< uint8_t > data_ )
{
  if ( m_CurrentState == STATE::STATE_COLLECTING )
  {
    std::cout << "SearchServer: Collecting - Received packet." << std::endl;

    std::string addr = AddrInfo::SockAddrToAddress( (sockaddr *) &client_ );
    PacketData data { addr, data_ };

    std::unique_lock< std::mutex > lck( m_mutex );
    m_PacketList.push_back( data );
  }
}
