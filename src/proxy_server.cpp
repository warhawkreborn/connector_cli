#include "addr_info.h"
#include "network.h"
#include "proxy_server.h"
#include "search_server.h"


ProxyServer::ProxyServer( ServerList &serverList_, PacketServer &server_, Network &network_ )
  : m_ServerList( serverList_ )
  , m_PacketServer( server_ )
  , m_Network( network_ )
{
  m_PacketServer.Register( this );
}


ProxyServer::~ProxyServer( )
{
  m_PacketServer.Unregister( this );
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

  // If we receive a packet from a remote server then turn ProxyMode ON.
  if ( !fromLocalNetwork && m_ServerListServer == "" )
  {
    m_ProxyMode = true;
    // Record the www.thalhammer.it site so we can distinguish between it
    // and the game client addresses.
    m_ServerListServer = fromIp;
  }

  if ( !m_ProxyMode )
  {
    return;
  }

  int debug = 0;

  if ( fromLocalNetwork )
  {
    if ( data_[ 0 ] == 0xc3 && data_[ 1 ] == 0x81 && data_.size( ) != 174 )
    {
      return; // This query packet which we are not intersted in.
    }

    // Broadcast it to the entire local LAN.
    AddrInfo sendAddr;
    sendAddr.SetAddr( m_ServerListServer );
    sendAddr.PortToSockAddr( m_PacketServer.GetServer().GetPort(), (sockaddr *) sendAddr.GetAiAddr() );
    const bool broadcast = false;
    m_PacketServer.send( *sendAddr.GetAiAddr(), data_, broadcast );
  }
  else
  {
    std::string foundLocalServer = "";

    // Forward remote client to local LAN server.
    m_ServerList.ForEachServer( [ & ] ( ServerEntry &entry_ )
    {
      bool continueOn = true;

      if ( entry_.m_LocalServer )
      {
        foundLocalServer = entry_.m_PacketData.m_address;
        continueOn = false;
      }

      return continueOn;
    } );

    if ( foundLocalServer == "" )
    {
      // Broadcast it to the entire local LAN.
      AddrInfo broadcastAddr;
      broadcastAddr.SetAddr( "255.255.255.255" );
      broadcastAddr.PortToSockAddr( m_PacketServer.GetServer( ).GetPort( ), (sockaddr *) broadcastAddr.GetAiAddr() );
      const bool broadcast = true;
      m_PacketServer.send( *broadcastAddr.GetAiAddr( ), data_, broadcast );
    }
    else
    {
      debug = 6;
    }
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
