#include <thread>

#include "addr_info.h"
#include "udp_server.h"
#include "search_server.h"
#include "warhawk_api.h"


SearchServer::SearchServer( ServerList &serverList_, PacketServer *packetServer_ )
  : m_ServerList( serverList_ )
  , m_mutex( )
  , m_PacketServer( packetServer_ )
  , m_CurrentState( STATE::STATE_BROADCASTING )
  , m_Thread( [&] ( ) { run( ); } )

{
  m_PacketServer->Register( this );
}


SearchServer::~SearchServer( )
{
  m_PacketServer->Unregister( this );
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
      std::string addr = AddrInfo::SockAddrToAddress( &client_ );
      PacketData item { addr, data_ };

      std::unique_lock< std::mutex > lck( m_mutex );
      m_PacketList.push_back( item );
    }
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
  clientAddr.PortToSockAddr( m_PacketServer->GetServer().GetPort(), (sockaddr *) clientAddr.GetAiAddr() );

  const std::vector< uint8_t > discoveryPacketData = m_PacketServer->hex2bin( m_DiscoveryPacket );

  try
  {
    // Broadcast Server Discovery Packet
    const bool broadcast = true;
    m_PacketServer->send( *clientAddr.GetAiAddr(), discoveryPacketData, broadcast );
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

  warhawk::API::ForwardingResponse publicIpResponse;

#ifdef LOGDATA
  std::cout << "SearchServer: Processing " << m_PacketList.size() << " packets." << std::endl;
#endif
  try
  {
    publicIpResponse = warhawk::API::CheckForwarding( );
  }
  catch ( const std::exception &e_ )
  {
    std::cout << "SearchServer: Couldn't perform CheckForwarding: " << e_.what() << std::endl;
  }

  std::unique_lock< std::mutex > lck( m_mutex );

  // If we can't get "online" status then abort.
  if ( publicIpResponse.m_state != "online" )
  {
    std::cout << "SearchServer: Server not online, check your port forwarding!" << std::endl;
    m_PacketList.clear( );
  }

  std::vector< ServerEntry > localServerList; // Build a new list of local servers.

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

    // Add the host to the remote server.
    auto addHostResponse = warhawk::API::AddHost( packetData.m_data.GetName( ), "", false );

    // Add the host to our list of Local Servers.
    ServerEntry lsdata;
    lsdata.m_PacketData        = packetData;
    lsdata.m_PublicIpResponse  = publicIpResponse;
    lsdata.m_LocalServer       = true;

    localServerList.push_back( lsdata );
  }

  // Update the information from the packet into the local server list.
  m_ServerList.AddLocalServerEntries( localServerList );

  m_CurrentState = STATE::STATE_WAITING;
}


bool SearchServer::LocalServerContainsIp( const std::string &ip_ )
{
  bool found = false;

  m_ServerList.ForEachServer( [ & ] ( const ServerEntry &localData_ )
  {
    bool continueOn = true;

    if ( ip_ == localData_.m_PublicIpResponse.m_ip )
    {
      found = true;
      continueOn = false;
    }

    return continueOn;
  } );

  return found;
}
