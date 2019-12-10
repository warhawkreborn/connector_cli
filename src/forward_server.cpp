#include "forward_server.h"

ForwardServer::ForwardServer( Server *server_ )
  : m_mutex( )
  , m_entries( )
  , m_server( server_ )
{
  m_server->Register( this );
}


ForwardServer::~ForwardServer( )
{
  m_server->Unregister( this );
}


void ForwardServer::SetEntries( std::vector< ServerEntry > e_ )
{
  std::unique_lock< std::mutex > lck( m_mutex );
  m_entries = std::move( e_ );
}


void ForwardServer::OnReceivePacket( sockaddr_storage client_, std::vector< uint8_t > data_ )
{
  if ( data_.size( ) > 300)
  {
    // This is not a request, but a response, so we exit early.
    return;
  }
  std::cout << "ForwardServer: Received packet." << std::endl;

  if ( !valid_packet( data_ ) )
  {
    std::cout << "ForwardServer: Received invalid frame, skipping" << std::endl;
    return;
  }

  if ( data_[ 0 ] == 0xc3 && data_[ 1 ] == 0x81 )
  {
    std::cout << "ForwardServer: Sending server list" << std::endl;

    std::unique_lock< std::mutex > lck( m_mutex );

    for ( auto &e : m_entries )
    {
      m_server->send( client_, e.m_frame );
    }
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
