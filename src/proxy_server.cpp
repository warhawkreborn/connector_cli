#include <algorithm>
#include <string>
#include <sstream>

#include "addr_info.h"
#include "network.h"
#include "packet_processor.h"
#include "proxy_server.h"
#include "search_server.h"
#include "warhawk.h"


ProxyServer::ProxyServer( ServerList &serverList_, PacketProcessor &packetProcessor_, Network &network_ )
  : m_ServerList( serverList_ )
  , m_PacketProcessor( packetProcessor_ )
  , m_Network( network_ )
{
  std::cout << "Starting ProxyServer..." << std::endl;

  std::vector< std::string > addressList;
  addressList = m_Network.ResolveIpAddress( WARHAWK_SERVER_LIST_SERVER );

  if ( addressList.size( ) < 1 )
  {
    std::stringstream ss;
    ss << "Can't resolve IP address for WARHAWK_SERVER_LIST_SERVER: " << WARHAWK_SERVER_LIST_SERVER;
    throw std::runtime_error( ss.str( ) );
  }

  m_ServerListServer = addressList[ 0 ];

  m_PacketProcessor.Register( this,
     ( (int) Packet::TYPE::TYPE_SERVER_INFO_REQUEST  |
       (int) Packet::TYPE::TYPE_SERVER_INFO_RESPONSE |
       (int) Packet::TYPE::TYPE_GAME_CLIENT_TO_SERVER|
       (int) Packet::TYPE::TYPE_GAME_SERVER_TO_CLIENT ) );
}


ProxyServer::~ProxyServer( )
{
  m_PacketProcessor.Unregister( this );
}


void ProxyServer::OnReceivePacket( const Packet &packet_ )
{
#ifdef LOGDATA
  std::cout << "ProxyServer: Received packet." << std::endl;
#endif

  // If we receive a packet from a WARHAWK_SERVER_LIST_SERVER then turn ProxyMode ON.
  if ( packet_.GetFromIp( ) == m_ServerListServer )
  {
    m_LastServerListServerPort = AddrInfo::SockAddrToPort( (const sockaddr *) &packet_.GetClient( ) );
    m_ProxyMode = true;
  }

  if ( !m_ProxyMode )
  {
    return;
  }

  if ( packet_.GetFromLocalNetwork( ) )
  {
    if ( packet_.GetType( ) == Packet::TYPE::TYPE_GAME_SERVER_TO_CLIENT )
    {
      OnHandleGameServerToClient( packet_ );
      return;
    }

    if ( !m_ReplyingToQuery )
    {
      return;
    }

    m_ReplyingToQuery = false;

    if ( packet_.GetType( ) == Packet::TYPE::TYPE_SERVER_INFO_RESPONSE )
    {
      OnHandleServerInfoResponse( packet_ );
    }
  }
  else
  {
    if ( packet_.GetType( ) == Packet::TYPE::TYPE_SERVER_INFO_REQUEST )
    {
      OnHandleServerInfoRequest( packet_ );
    }
    else if ( packet_.GetType( ) == Packet::TYPE::TYPE_GAME_CLIENT_TO_SERVER )
    {
      OnHandleGameClientToServer( packet_ );
    }
  }
}


void ProxyServer::OnHandleServerInfoRequest( const Packet &packet_ )
{
  int debug = 0;

  std::string localServerIp = "";

  // Forward remote client to local LAN server.
  m_ServerList.ForEachServer( [ & ] ( ServerEntry &entry_ )
  {
    bool continueOn = true;

    if ( entry_.m_LocalServer )
    {
        localServerIp = entry_.m_PacketData.m_address;
        continueOn = false;
    }

    return continueOn;
  } );

  if ( localServerIp != "" )
  {
    // Send it to the local server.
    AddrInfo localServerAddr;
    localServerAddr.SetAddr( localServerIp );
    localServerAddr.PortToSockAddr( WARHAWK_UDP_PORT, (sockaddr *) localServerAddr.GetAiAddr() );
    const bool broadcast = false;
    m_PacketProcessor.send( *localServerAddr.GetAiAddr( ), packet_.GetData( ), broadcast );

    // std::cout << "ProxyServer::OnHandleInfoRequest - Sending packet to local server at " << localServerIp << std::endl;

    if ( packet_.GetFromIp( ) == m_ServerListServer )
    {
      m_ReplyingToQuery = true;
    }
  }
  else
  {
    debug = 6;
  }
}


void ProxyServer::OnHandleServerInfoResponse( const Packet &packet_ )
{
  // Send it to the WarHawkServerListServer.
  AddrInfo sendAddr;
  sendAddr.SetAddr( m_ServerListServer );
  sendAddr.PortToSockAddr( m_LastServerListServerPort, (sockaddr *) sendAddr.GetAiAddr( ) );
  const bool broadcast = false;

  bool foundPublicServerIp = false;
  std::array< uint8_t, 4 > publicServerIp;

  m_ServerList.ForEachServer( [ &, this ] ( ServerEntry &entry_ )
    {
      bool continueOn = true;

      if ( entry_.m_LocalServer )
      {
        foundPublicServerIp = true;
        publicServerIp = warhawk::net::UdpNetworkSocket::StringToIp( entry_.m_ip );
        continueOn = false;
      }

      return continueOn;
    } );

  if ( !foundPublicServerIp )
  {
    return;
  }

  Packet newPacket( packet_.GetData( ), true );

  // Fill in our public server address instead of the local server address.
  uint8_t *dataPtr = static_cast< uint8_t * >( &(newPacket.GetData( )[ 0 ]) );
  auto frame = reinterpret_cast< warhawk::net::server_info_response * >( dataPtr + 4 );

  memcpy( frame->m_ip1, &publicServerIp, sizeof( frame->m_ip1 ) );
  memcpy( frame->m_ip2, &publicServerIp, sizeof( frame->m_ip2 ) );

  m_PacketProcessor.send( *sendAddr.GetAiAddr( ), packet_.GetData( ), broadcast );
}


void ProxyServer::OnHandleGameClientToServer( const Packet &packet_ )
{
  ClientIpList::iterator itr = m_ClientIpList.find( packet_.GetFromIp( ) );

  if ( itr == m_ClientIpList.end( ) )
  {
    // It is not on the list, so put it there.
    ClientData data;
    data.m_PacketFromClient = packet_;
    m_ClientIpList[ packet_.GetFromIp( ) ] = data;
    itr = m_ClientIpList.find( packet_.GetFromIp( ) );
  }
  else
  {
    // Update it it on the list.
    ClientData &data = itr->second;
    data.m_PacketFromClient = packet_;
  }

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
  sendAddr.PortToSockAddr( WARHAWK_UDP_PORT, (sockaddr *) sendAddr.GetAiAddr( ) );
  const bool broadcast = false;
  m_PacketProcessor.send( *sendAddr.GetAiAddr( ), packet_.GetData( ), broadcast );

  std::cout << "CLIENT_TO_SERVER: Length=" << packet_.GetData( ).size( ) << std::endl;
}


void ProxyServer::OnHandleGameServerToClient( const Packet &packet_ )
{
  for ( const auto &client : m_ClientIpList )
  {
    if ( true ) // This needs to somehow differentiate between clients, but just one for now.
    {
      const std::string &clientIp = client.first;
      // Send it to the client.
      AddrInfo sendAddr;
      sendAddr.SetAddr( clientIp );
      sendAddr.PortToSockAddr( WARHAWK_UDP_PORT, (sockaddr *) sendAddr.GetAiAddr( ) );
      const bool broadcast = false;
      m_PacketProcessor.send( *sendAddr.GetAiAddr( ), packet_.GetData( ), broadcast );

      std::cout << "SERVER_TO_CLIENT: Length=" << packet_.GetData( ).size( ) << std::endl;

      break;
    }
  }
}
