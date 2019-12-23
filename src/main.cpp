//
// This program acts as an intermediary which does several things
// 1. Gets a list of current remote warhawk servers from https://warhawk.thalhammer.it/api/ URL.
// 2. Broadcasts that list of servers to a warhawk client when it requests the list of servers.
// 3. Watches for warhawk local warhawk servers on the network and sends those so the remote server.
//

#include <sstream>
#include <thread>

#include "forward_server.h"
#include "search_server.h"
#include "warhawk.h"
#include "warhawk_api.h"

// Macros
#define STRINGIFY_( x ) #x
#define STRINGIFY( x ) STRINGIFY_( x )


// Generate string version.
std::string VersionString( )
{
  std::stringstream ss;
  ss << "WarHawk Reborn Version " << STRINGIFY( PROJECT_VERSION ) << "-";
  ss << STRINGIFY( GIT_HASH );
  ss << " (" << __DATE__ << " " << __TIME__ << ")";

  return ss.str( );
}


// Main program
int main( int argc_, const char **argv_ )
{
  // Check to see whether we should parse any command-line arguments.
  if ( argc_ > 1 )
  {
    std::string option = argv_[ 1 ];

    // Check for -v or --version commandline argument.
    if ( option == "-v" || option == "--version" )
    {
      std::cout << VersionString( ) << std::endl;

      return 0;
    }

    std::cerr << "Unknown option." << std::endl;
    return 1;
  }

  std::cout << VersionString( ) << std::endl;;

  std::cout << "Warhawk bridge booting..." << std::endl;

  try
  {
    // Set up to listen for UDP packets on standard WarHawk port.
    warhawk::net::udp_server udpServer( WARHAWK_UDP_PORT );

    // 
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
        forwardServer.SetEntries( list );
        searchServer.SetEntries(list);

        std::cout << "MainLoop: " << list.size() << " servers found" << std::endl;

        for ( auto &e : list )
        {
          std::cout << "MainLoop: " << e.m_name << " " << e.m_ping << "ms" << std::endl;
        }
      }

      std::this_thread::sleep_for( std::chrono::seconds( 60 ) );
    }
  }
  catch ( const std::exception e_ )
  {
    std::cout << "Error: " << e_.what( ) << std::endl;
    return -1;
  }

  return 0;
}
