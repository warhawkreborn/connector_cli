#include "addr_info.h"
#include "forward_server.h"
#include "network.h"
#include "search_server.h"


ForwardServer::ForwardServer( ServerList &serverList_, PacketProcessor &packetProcessor_, Network &network_ )
  : m_ServerList( serverList_ )
  , m_PacketProcessor( packetProcessor_ )
  , m_Network( network_ )
{
  m_PacketProcessor.Register( this, (int) Packet::TYPE::TYPE_SERVER_INFO_REQUEST );
}


ForwardServer::~ForwardServer( )
{
  m_PacketProcessor.Unregister( this );
}


void ForwardServer::OnReceivePacket( const Packet & packet_ )
{

#ifdef LOGDATA
  std::cout << "ForwardServer: Received packet." << std::endl;
#endif

#ifdef LOGDATA
    std::cout << "ForwardServer: Sending server list" << std::endl;
#endif

  // This server only handles packets from the local network.
  if ( !packet_.GetFromLocalNetwork( ) )
  {
    return;
  }

  m_ServerList.ForEachServer( [ this, &packet_ ] ( const ServerEntry &entry_ )
  {
    // If it is from the local network then forward only remote servers to sender on local network.
    if ( !entry_.m_LocalServer )
    {
      m_PacketProcessor.send( packet_.GetClient( ), entry_.m_frame );
    }

    const bool continueOn = true;
    return continueOn;
  } );
}
