#include <sstream>
#include <thread>

#include "forward_server.h"
#include "search_server.h"
#include "warhawk.h"
#include "warhawk_api.h"
#include "version.h"

// Macros
#define STRINGIFY_( x ) #x
#define STRINGIFY( x ) STRINGIFY_( x )


// WarHawk Reborn Version
const std::string Version = "1.1.0";


// Generate string version.
std::string VersionString( )
{
  std::stringstream ss;
  ss << "WarHawk Reborn Version " << Version << "-";
#ifdef WARHAWK_BUILD
  ss << STRINGIFY( WARHAWK_BUILD );
#else
  ss << warhawk::Version::GIT_HASH;
#endif
  ss << " (" << warhawk::Version::GIT_DATE << ")";

  return ss.str( );
}


int main( int argc_, const char **argv_ )
{
  if ( argc_ > 1 )
  {
    std::string option = argv_[ 1 ];

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
