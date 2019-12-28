#include "forward_server.h"
#include "search_server.h"


ForwardServer::ForwardServer( ServerList &serverList_, PacketServer *server_ )
  : m_ServerList( serverList_ )
  , m_PacketServer( server_ )
{
  m_PacketServer->Register( this );
}


ForwardServer::~ForwardServer( )
{
  m_PacketServer->Unregister( this );
}


void ForwardServer::OnReceivePacket( sockaddr_storage client_, std::vector< uint8_t > data_ )
{
  if ( data_.size( ) > 300)
  {
    // This is not a request, but a response, so we exit early.
    return;
  }

#ifdef LOGDATA
  std::cout << "ForwardServer: Received packet." << std::endl;
#endif

  if ( !valid_packet( data_ ) )
  {
    std::cout << "ForwardServer: Received invalid frame, skipping" << std::endl;
    return;
  }

  if ( data_[ 0 ] == 0xc3 && data_[ 1 ] == 0x81 )
  {
#ifdef LOGDATA
    std::cout << "ForwardServer: Sending server list" << std::endl;
#endif

    m_ServerList.ForEachServer( [ this, client_ ] ( const ServerEntry &entry_ )
    {
      if ( !entry_.m_LocalServer )
      {
        m_PacketServer->send( client_, entry_.m_frame );
      }

      const bool continueOn = true;
      return continueOn;
    } );
  }
  else
  {
    std::cout << "ForwardServer: Unknown frame type, ignoring" << std::endl;
  }
}


bool ForwardServer::valid_packet( const std::vector< uint8_t > &data_ )
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
