#include <thread>

#include "forward_server.h"
#include "search_server.h"
#include "warhawk.h"
#include "warhawk_api.h"


int main( int argc_, const char **argv_ )
{
  std::cout << "Warhawk bridge booting..." << std::endl;

  warhawk::net::udp_server udpServer( WARHAWK_UDP_PORT );

  Server packetServer( udpServer );

  std::thread packetServerThread( [&]( )
  {
    std::cout << "Starting Packet Server..." << std::endl;
    packetServer.run( );
    std::cout << "Stopping Packet Server." << std::endl;
  } );

  ForwardServer forwardServer( &packetServer );

  SearchServer searchServer( &packetServer );

  std::thread searchServerThread( [&] ( )
  {
    std::cout << "Starting Search Server..." << std::endl;
    searchServer.run( );
    std::cout << "SearchServer thread ended." << std::endl;
  } );

  while ( true )
  {
    std::cout << "MainLoop: Updating server list" << std::endl;
    std::vector< ServerEntry > list;
    try
    {
      list = warhawk::API::DownloadServerList( &packetServer );
    }
    catch ( const std::exception &e_ )
    {
      std::cout << "MainLoop: Error = " << e_.what( ) << std::endl;
      std::cout << "MainLoop: Check network connection." << std::endl;
    }

    if ( list.size( ) > 0 )
    {
      forwardServer.set_entries( list );

      std::cout << "MainLoop: " << list.size() << " servers found" << std::endl;

      for ( auto &e : list )
      {
        std::cout << "MainLoop: " << e.m_name << " " << e.m_ping << "ms" << std::endl;
      }
    }

    std::this_thread::sleep_for( std::chrono::seconds( 60 ) );
  }

  return 0;
}
