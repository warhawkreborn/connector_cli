#include <thread>

#include "addr_info.h"
#include "net.h"
#include "search_server.h"
#include "warhawk_api.h"


SearchServer::SearchServer( Server *server_ )
  : m_mutex( )
  , m_entries( )
  , m_server( server_ )
  , m_CurrentState( STATE::STATE_BROADCASTING )
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

        m_CurrentState = STATE::STATE_BROADCASTING;

        break;
      }

      case STATE::STATE_BROADCASTING:
      {
        std::cout << "SearchServer: Broadcasting - Searching for new servers to publish." << std::endl;

        try
        {
          // Broadcast Server Discovery Packet
          const bool broadcast = true;
          m_server->send( *clientAddr.GetAiAddr( ), discoveryPacketData, broadcast );
        }
        catch ( const std::exception &e_ )
        {
          std::cout << "SearchServer: Error trying to broadcast packet: " << e_.what( ) << std::endl;
        }

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

            warhawk::API::ForwardingResponse response;

            try
            {
              response = warhawk::API::CheckForwarding( );
            }
            catch ( const std::exception &e_ )
            {
              std::cout << "SearchServer: Couldn't perform CheckForwarding: " << e_.what( ) << std::endl;
            }

            for(auto & entry : m_entries)
            {
              if(entry.m_ip == response.m_ip)
              {
                // Skipping AddHost as it already exists
                m_PacketList.clear();
                break;
              }
            }
            
            for ( PacketList::iterator itr = m_PacketList.begin( ); itr != m_PacketList.end( ); )
            {
              PacketData &data = *itr;
              std::cout << "SearchServer: " <<
                "State = '"            << response.m_state           << "', " <<
                "Public server IP = '" << response.m_ip              << "', " <<
                "Local  server IP = '" << data.m_address             << "', " <<
                "Name = '"             << data.m_data.GetName( )     << "', " <<
                "MapName = '"          << data.m_data.GetMapName( )  << "', " <<
                "GameMode = '"         << data.m_data.GetGameMode( ) << "'"   <<std::endl;

              auto response = warhawk::API::AddHost( data.m_data.GetName( ), "", false );

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

void SearchServer::SetEntries( std::vector< ServerEntry > e_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );
  m_entries = std::move( e_ );
}