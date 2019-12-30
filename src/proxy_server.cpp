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

#ifdef FUTURE
  if ( data_[ 0 ] == 0xc3 && data_[ 1 ] == 0x81 )
  {
#ifdef LOGDATA
    std::cout << "ProxyServer: Sending server list" << std::endl;
#endif

    std::string fromIp = AddrInfo::SockAddrToAddress( &client_ );
    bool fromLocalNetwork = m_Network.OnLocalNetwork( fromIp );

    // If we receive a packet from a remote server then turn ProxyMode ON.
    if ( !fromLocalNetwork )
    {
      m_ProxyMode = true;
    }

    if ( !m_ProxyMode )
    {
      return;
    }

    m_ServerList.ForEachServer( [ this, &client_, fromLocalNetwork ] ( const ServerEntry &entry_ )
    {
      // If it is from the local network then forward only remote servers to sender on local network.
      if ( !entry_.m_LocalServer )
      {
        m_PacketServer.send( client_, entry_.m_frame );
      }

      const bool continueOn = true;
      return continueOn;
    } );
  }
  else
  {
    std::cout << "ProxyServer: Unknown frame type, ignoring" << std::endl;
  }
#endif
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
