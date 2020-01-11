#include <algorithm>
#include <memory>
#include <string>
#include <string.h>
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
  std::cout << "ProxyServer: Starting..." << std::endl;

  std::vector< std::string > addressList;
  addressList = m_Network.ResolveIpAddress( WARHAWK_SERVER_LIST_SERVER );

  if ( addressList.size( ) < 1 )
  {
    std::stringstream ss;
    ss << "ProxyServer: Can't resolve IP address for WARHAWK_SERVER_LIST_SERVER: " << WARHAWK_SERVER_LIST_SERVER;
    throw std::runtime_error( ss.str( ) );
  }

  m_ServerListServer = addressList[ 0 ];

  m_PacketProcessor.Register( this,
     ( (int) Packet::TYPE::TYPE_SERVER_INFO_REQUEST  |
       (int) Packet::TYPE::TYPE_SERVER_INFO_RESPONSE |
       (int) Packet::TYPE::TYPE_GAME_CLIENT_TO_SERVER ) );
}


ProxyServer::~ProxyServer( )
{
  m_PacketProcessor.Unregister( this );

  std::cout << "ProxyServer: Stopping." << std::endl;
}


void ProxyServer::OnReceivePacket( const Packet &packet_ )
{
#ifdef LOGDATA
  std::cout << "ProxyServer: Received packet." << std::endl;
#endif

  // Check to see if there are any disonnections
  ManageClientList( );

  // If we receive a packet from a WARHAWK_SERVER_LIST_SERVER then turn ProxyMode ON.
  if ( packet_.GetIp( ) == m_ServerListServer )
  {
    m_LastServerListServerPort = AddrInfo::SockAddrToPort( &packet_.GetClient( ) );

    if ( !m_ProxyMode )
    {
      m_ProxyMode = true;
      std::cout << "Proxyserver - ProxyMode detected: Turning ProxyMode = ON" << std::endl;
    }
  }

  if ( !m_ProxyMode )
  {
    return;
  }

  if ( packet_.GetFromLocalNetwork( ) )
  {
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
    localServerAddr.SetPort( WARHAWK_UDP_PORT );
    const bool broadcast = false;
    m_PacketProcessor.send( *localServerAddr.GetAiAddr( ), packet_.GetData( ), broadcast );

    // std::cout << "ProxyServer::OnHandleInfoRequest - Sending packet to local server at " << localServerIp << std::endl;

    if ( packet_.GetIp( ) == m_ServerListServer )
    {
      m_ReplyingToQuery = true;
    }
  }
}


void ProxyServer::OnHandleServerInfoResponse( const Packet &packet_ )
{
  // Send it to the WarHawkServerListServer.
  AddrInfo sendAddr;
  sendAddr.SetAddr( m_ServerListServer );
  sendAddr.SetPort( m_LastServerListServerPort );
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
  ClientList::iterator itr = std::find_if( m_ClientList.begin( ), m_ClientList.end( ), [ &packet_ ] ( ClientServerPtr &clientServer_ )
  {
    return clientServer_->GetPublicIp( ) == packet_.GetIp( ) &&
           clientServer_->GetPublicPort( ) == packet_.GetPort( );
  } );

  if ( itr == m_ClientList.end( ) )
  {
    // It is not on the list, so put it there.
    uint16_t port = AddrInfo::SockAddrToPort( &packet_.GetClient( ) );

    // Set up to manage connection between client and WarHawk Server.
    // This will also receive packets from the WarHawk Server and forward them to the client.
    std::stringstream name;
    name << "Player <" << packet_.GetIp( ) << ":" << port << ">";
    ClientServerPtr newClientServer = std::make_unique< ClientServer >( name.str( ), packet_.GetIp( ), port, m_ServerList, m_Network, m_PacketProcessor );

    if ( m_ClientList.size( ) < WARHAWK_MAX_PLAYERS )
    {
      m_ClientList.push_back( std::move( newClientServer ) );

      itr = std::find_if( m_ClientList.begin( ), m_ClientList.end( ), [ &packet_ ] ( ClientServerPtr &clientServer_ )
      {
        return clientServer_->GetPublicIp( ) == packet_.GetIp( ) &&
               clientServer_->GetPublicPort( ) == packet_.GetPort( );
      } );

      std::cout << name.str( ) << " joined game." << std::endl;
    }
    else
    {
      std::cout << "ProxyServer: MAX PLAYERS - Client join rejected from IP=" << packet_.GetIp( ) <<
        ", Port=" << port << std::endl;
      return;
    }

    std::cout << "ProxyServer: Total Current Clients = " << m_ClientList.size( ) << std::endl;
  }

  if ( itr != m_ClientList.end( ) )
  {
    ClientServerPtr &clientServer = *itr;

    clientServer->SendPacket( packet_ ); // Send Packet to WarHawk Server.
  }
}


void ProxyServer::ManageClientList( )
{
  using namespace std::chrono_literals;
  using seconds = std::chrono::seconds;

  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now( );;

  for ( ClientList::iterator itr = m_ClientList.begin( );
        itr != m_ClientList.end( );
        )
  {
    ClientServerPtr &player = *itr;
    auto notPlayingFor = std::chrono::duration_cast< seconds >( now - player->GetLastPacketTime( ) );

    if ( notPlayingFor > 10s )
    {
      std::cout << "ProxyServer: " << player->GetName( ) << " inactive for > 10 seconds: Deleting." << std::endl;
      itr = m_ClientList.erase( itr );
      std::cout << "ProxyServer: Total Current Clients = " << m_ClientList.size( ) << std::endl;
    }
    else
    {
      ++itr;
    }
  }
}
