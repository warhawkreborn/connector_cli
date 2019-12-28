#include <thread>

#include "addr_info.h"
#include "udp_server.h"
#include "search_server.h"
#include "warhawk_api.h"


SearchServer::SearchServer( Server *server_ )
  : m_mutex( )
  , m_entries( )
  , m_server( server_ )
  , m_CurrentState( STATE::STATE_BROADCASTING )
  , m_Thread( [&] ( ) { run( ); } )

{
  m_server->Register( this );
}


SearchServer::~SearchServer( )
{
  m_server->Unregister( this );
  m_Done = true;
  m_Thread.join( );
}


void SearchServer::run( )
{
  std::cout << "Starting Search Server..." << std::endl;

  while ( !m_Done )
  {
    switch ( m_CurrentState )
    {
      case STATE::STATE_WAITING:
        DoStateWaiting( );
        break;

      case STATE::STATE_BROADCASTING:
        DoStateBroadcasting( );
        break;

      case STATE::STATE_COLLECTING:
        DoStateCollecting( );
        break;

      case STATE::STATE_PROCESSING:
        DoStateProcessing( );
        break;
    }
  }

  std::cout << "SearchServer thread ended." << std::endl;
}


void SearchServer::OnReceivePacket( struct sockaddr_storage client_, std::vector< uint8_t > data_ )
{
  if ( m_CurrentState == STATE::STATE_COLLECTING )
  {
#ifdef LOGDATA
    std::cout << "SearchServer: Collecting - Received packet." << std::endl;
#endif

    // The packet received from the PS3 is 372 bytes in length.
    // Anything else is probably the discovery packet.
    if ( data_.size( ) == 372 )
    {
      std::string addr = AddrInfo::SockAddrToAddress( (sockaddr *) &client_ );
      PacketData item { addr, data_ };

      std::unique_lock< std::mutex > lck( m_mutex );
      m_PacketList.push_back( item );
    }
  }
}

void SearchServer::SetEntries( std::vector< ServerEntry > e_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );
  m_entries = std::move( e_ );
}


void SearchServer::ForEachServer( std::function< void( const LocalServerData & ) > func_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );

  ForEachServerNoLock( func_ );
}


void SearchServer::ForEachServerNoLock( std::function< void( const LocalServerData & ) > func_ )
{
  for ( const auto &entry : m_LocalServers )
  {
    func_( entry );
  }
}


void SearchServer::DoStateWaiting( )
{
  std::this_thread::sleep_for( std::chrono::seconds( 30 ) );

  m_CurrentState = STATE::STATE_BROADCASTING;
}


void SearchServer::DoStateBroadcasting( )
{
#ifdef LOGDATA
  std::cout << "SearchServer: Broadcasting - Searching for new servers to publish." << std::endl;
#endif

  AddrInfo clientAddr;
  clientAddr.SetAddr( "255.255.255.255" );
  clientAddr.PortToSockAddr( m_server->GetServer().GetPort(), (sockaddr *) clientAddr.GetAiAddr() );

  const std::vector< uint8_t > discoveryPacketData = m_server->hex2bin( m_DiscoveryPacket );

  try
  {
    // Broadcast Server Discovery Packet
    const bool broadcast = true;
    m_server->send( *clientAddr.GetAiAddr(), discoveryPacketData, broadcast );
  }
  catch ( const std::exception &e_ )
  {
    std::cout << "SearchServer: Error trying to broadcast packet: " << e_.what() << std::endl;
  }

  m_CurrentState = STATE::STATE_COLLECTING;
}


void SearchServer::DoStateCollecting( )
{
  std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

  m_CurrentState = STATE::STATE_PROCESSING;
}


void SearchServer::DoStateProcessing( )
{
  // Now do something with any packets collected.

#ifdef LOGDATA
  std::cout << "SearchServer: Processing any collected packets." << std::endl;
#endif

  warhawk::API::ForwardingResponse response;

#ifdef LOGDATA
  std::cout << "SearchServer: Processing " << m_PacketList.size() << " packets." << std::endl;
#endif
  try
  {
    response = warhawk::API::CheckForwarding();
  }
  catch ( const std::exception &e_ )
  {
    std::cout << "SearchServer: Couldn't perform CheckForwarding: " << e_.what() << std::endl;
  }

  std::unique_lock< std::mutex > lck( m_mutex );

  for ( PacketList::iterator itr = m_PacketList.begin();
        itr != m_PacketList.end( );
        itr = m_PacketList.erase( itr ) )
  {
    PacketData &packetData = *itr;

#ifdef LOGDATA
    std::cout << "SearchServer: "
              << "State = '"            << response.m_state                 << "', "
              << "Public server IP = '" << response.m_ip                    << "', "
              << "Local  server IP = '" << packetData.m_address             << "', "
              << "Name = '"             << packetData.m_data.GetName(     ) << "', "
              << "MapName = '"          << packetData.m_data.GetMapName(  ) << "', "
              << "GameMode = '"         << packetData.m_data.GetGameMode( ) << "'" << std::endl;
#endif

    LocalServerList::iterator litr = std::find_if( m_LocalServers.begin( ), m_LocalServers.end( ), [&] ( LocalServerData &entry_ )
    {
      // Return true if match.
      return entry_.m_Response.m_ip == response.m_ip;
    } );
    
    if ( litr == m_LocalServers.end( ) )
    {
      // If it is not on the list then check the status.
      if ( response.m_state != "online" )
      {
        std::cout << "SearchServer: Server not online, check your port forwarding!" << std::endl;
        continue;
      }

      // Add the host to the remote server.
      auto addHostResponse = warhawk::API::AddHost( packetData.m_data.GetName( ), "", false );

      // Add the host to our m_LocalServers.
      LocalServerData lsdata( packetData, response );

      m_LocalServers.push_back( lsdata );
    }
    else
    {
      // Update the information in the list from the packet.
      litr->m_PacketData = packetData;
      litr->m_Response   = response;
    }
  }

  m_CurrentState = STATE::STATE_WAITING;
}

bool SearchServer::LocalServerContainsIp( const std::string &ip_ )
{
  bool found = false;

  ForEachServer( [ & ] ( const LocalServerData &localData_ )
  {
    if ( ip_ == localData_.m_Response.m_ip )
    {
      found = true;
    }
  } );

  return found;
}
