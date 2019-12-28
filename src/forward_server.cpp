#include "forward_server.h"
#include "search_server.h"


ForwardServer::ForwardServer( PacketServer *server_, SearchServer &searchServer_ )
  : m_mutex( )
  , m_entries( )
  , m_PacketServer( server_ )
  , m_SearchServer( searchServer_ )
{
  m_PacketServer->Register( this );
}


ForwardServer::~ForwardServer( )
{
  m_PacketServer->Unregister( this );
}


void ForwardServer::SetEntries( std::vector< ServerEntry > serverEntries_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );

  m_entries.clear( );

  for ( const auto &sourceEntry : serverEntries_ )
  {
    if ( !m_SearchServer.LocalServerContainsIp( sourceEntry.m_ip ) )
    {
      m_entries.push_back( sourceEntry );
    }
  }
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

    ForEachServer( [ this, client_ ] ( auto e )
    {
      m_PacketServer->send( client_, e.m_frame );
    } );
  }
  else
  {
    std::cout << "ForwardServer: Unknown frame type, ignoring" << std::endl;
  }
}


void ForwardServer::ForEachServer( std::function< void ( const ServerEntry & ) > func_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );

  for ( auto &e : m_entries )
  {
    func_( e );
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
