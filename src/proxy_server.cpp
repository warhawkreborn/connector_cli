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

  m_PacketProcessor.Register( this );
}


ProxyServer::~ProxyServer( )
{
  m_PacketProcessor.Unregister( this );
}


void ProxyServer::OnReceivePacket( sockaddr_storage client_, std::vector< uint8_t > data_ )
{
#ifdef LOGDATA
  std::cout << "ProxyServer: Received packet." << std::endl;
#endif

  if ( !valid_packet( data_ ) )
  {
    std::cout << "ProxyServer: Received invalid frame, skipping" << std::endl;
    return;
  }

  std::string fromIp = AddrInfo::SockAddrToAddress( &client_ );
  bool fromLocalNetwork = m_Network.OnLocalNetwork( fromIp );

  // If we receive a packet from a WARHAWK_SERVER_LIST_SERVER then turn ProxyMode ON.
  if ( fromIp == m_ServerListServer )
  {
    m_LastServerListServerPort = AddrInfo::SockAddrToPort( (const sockaddr *) &client_ );
    m_ProxyMode = true;
  }

  if ( !m_ProxyMode )
  {
    return;
  }

  if ( fromLocalNetwork )
  {
    if ( !m_ReplyingToQuery )
    {
      return;
    }

    m_ReplyingToQuery = false;

    if ( data_.size( ) == 372 )
    {
      OnHandleServerInfoResponse( data_ );
    }
  }
  else
  {
    OnHandleServerInfoRequest( fromIp, data_ );
  }
}


bool ProxyServer::valid_packet( const std::vector< uint8_t > &data_ )
{
  if ( data_.size( ) < 4 )
  {
    return false;
  }

  uint16_t len = data_[ 3 ];
  len = ( len << 8 ) | data_[ 2 ];

  if ( data_.size( ) - 4 != len )
  {
    return false;
  }

  return true;
}


void ProxyServer::OnHandleServerInfoRequest( const std::string &fromIp_, std::vector< uint8_t > data_ )
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
    m_PacketProcessor.send( *localServerAddr.GetAiAddr( ), data_, broadcast );

    // std::cout << "ProxyServer::OnHandleInfoRequest - Sending packet to local server at " << localServerIp << std::endl;

    if ( fromIp_ == m_ServerListServer )
    {
      m_ReplyingToQuery = true;
    }
  }
  else
  {
    debug = 6;
  }
}


void ProxyServer::OnHandleServerInfoResponse( std::vector< uint8_t > data_ )
{
#if 0
  if ( data_[ 0 ] == 0xc3 && data_[ 1 ] == 0x81 && data_.size( ) != 174 )
  {
    return; // This query packet which we are not intersted in.
  }
#endif
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

  // Fill in our public server address instead of the local server address.
  auto frame = reinterpret_cast< warhawk::net::server_info_response * >( &data_ + 4 );

  memcpy( frame->m_ip1, &publicServerIp, sizeof( frame->m_ip1 ) );
  memcpy( frame->m_ip2, &publicServerIp, sizeof( frame->m_ip2 ) );

  m_PacketProcessor.send( *sendAddr.GetAiAddr(), data_, broadcast );
}
