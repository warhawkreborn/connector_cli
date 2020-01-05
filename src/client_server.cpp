#include "client_server.h"
#include "network.h"
#include "packet_processor.h"
#include "warhawk.h"

ClientServer::ClientServer( const std::string &name_,
                            const std::string &publicIpAddress_,
                            const int &publicPort_,
                            ServerList &serverList_,
                            Network &network_,
                            PacketProcessor &proxyToClientPacketProcessor_ )
  : m_Name( name_ )
  , m_PublicIpAddress( publicIpAddress_ )
  , m_PublicPort( publicPort_ )
  , m_ServerList( serverList_ )
  , m_Network( network_ )
  , m_ProxyToClientPacketProcessor( proxyToClientPacketProcessor_ )
  , m_ProxyToServerSocket( std::make_unique< warhawk::net::UdpNetworkSocket >( m_Network, 0 ) ) // Port 0 = choose a random port.
  , m_ProxyToServerPacketProcessor( std::make_unique< PacketProcessor >( name_, *m_ProxyToServerSocket, network_ ) )
{
  m_ProxyToServerPacketProcessor->Register( this, (int) Packet::TYPE::TYPE_GAME_SERVER_TO_CLIENT );
}


ClientServer::~ClientServer( )
{
  m_ProxyToServerPacketProcessor->Unregister( this );
}


std::string ClientServer::GetName( )
{
  return m_Name;
}


std::string ClientServer::GetPublicIp( )
{
  return m_PublicIpAddress;
}


// Send packet to WarHawk Server
void ClientServer::SendPacket( const Packet &packet_ )
{
  m_LastPacketTime = std::chrono::steady_clock::now( );

  std::string localServerIp;

  m_ServerList.ForEachServer( [ &, this ] ( ServerEntry &entry_ )
  {
    bool continueOn = true;

    if ( entry_.m_LocalServer )
    {
      localServerIp = entry_.m_PacketData.m_address; // LOCAL IP not PUBLIC.
      continueOn = false;
    }

    return continueOn;
  } );

  if ( localServerIp == "" )
  {
    return;
  }

  // Send it to the local WarHawkServer.
  AddrInfo sendAddr;
  sendAddr.SetAddr( localServerIp );
  sendAddr.SetPort( WARHAWK_UDP_PORT );
  const bool broadcast = false;
  m_ProxyToServerPacketProcessor->send( *sendAddr.GetAiAddr( ), packet_.GetData( ), broadcast );
#if 0
  std::cout << "CLIENT_TO_SERVER: IP=" << m_PublicIpAddress << ", Port=" << m_PublicPort << ", Length=" << packet_.GetData( ).size( ) << std::endl;
#endif
}


void ClientServer::OnReceivePacket( const Packet &packet_ )
{
  m_LastPacketTime = std::chrono::steady_clock::now( );

  uint16_t proxyPort = AddrInfo::SockAddrToPort( &packet_.GetClient( ) );

  // Send it to the client.
  AddrInfo sendAddr;
  sendAddr.SetAddr( m_PublicIpAddress );
  sendAddr.SetPort( m_PublicPort );
  const bool broadcast = false;
  m_ProxyToClientPacketProcessor.send( *sendAddr.GetAiAddr( ), packet_.GetData( ), broadcast );
#if 0
  std::cout << "SERVER_TO_CLIENT: IP=" << m_PublicIpAddress << ", Port=" << m_PublicPort << ", Length=" << packet_.GetData( ).size( ) << std::endl;
#endif
}


std::chrono::steady_clock::time_point ClientServer::GetLastPacketTime( )
{
  return m_LastPacketTime;
}
