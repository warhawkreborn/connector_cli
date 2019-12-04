#include "forward_server.h"


ForwardServer::ForwardServer( warhawk::net::udp_server &udpServer_ ) : m_server( udpServer_ )
{
}


void ForwardServer::set_entries( std::vector< ServerEntry > e_ )
{
  std::unique_lock< std::mutex > lck( m_mtx );
  m_entries = std::move( e_ );
}


void ForwardServer::run( )
{
  struct sockaddr_in client;
  std::vector< uint8_t > data;

  while ( m_server.receive( client, data ) )
  {
    if ( !valid_packet( data ) )
    {
      std::cout << "ForwardServer: Received invalid frame, skipping" << std::endl;
      continue;
    }

    if ( data[ 0 ] == 0xc3 && data[ 1 ] == 0x81 )
    {
      std::cout << "ForwardServer: Sending server list" << std::endl;
      std::unique_lock< std::mutex > lck( m_mtx );

      for ( auto &e : m_entries )
      {
        m_server.send( client, e.m_frame );
      }
    }
    else
    {
      std::cout << "ForwardServer: Unknown frame type, ignoring" << std::endl;
    }
  }
}


bool ForwardServer::valid_packet( const std::vector< uint8_t > &data_ )
{
  if ( data_.size() < 4 )
  {
    return false;
  }

  uint16_t len = data_[ 3 ];
  len = ( len << 8 ) | data_[ 2 ];

  if ( data_.size() - 4 != len )
  {
    return false;
  }

  return true;
}
