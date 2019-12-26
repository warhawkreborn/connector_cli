//
// This program acts as an intermediary which does several things
// 1. Gets a list of current remote warhawk servers from https://warhawk.thalhammer.it/api/ URL.
// 2. Broadcasts that list of servers to a warhawk client when it requests the list of servers.
// 3. Watches for warhawk local warhawk servers on the network and sends those so the remote server.
// 4. Provides a web server to show the list of servers and other information.
//

// System includes
#include <filesystem>
#include <sstream>
#include <thread>

#ifdef WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

// uWebSockets includes
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4267 )
#else
#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-w"
#endif

#include "App.h"

#ifdef WIN32
#pragma warning( pop )
#else
#pragma GCC diagnostic pop
#endif

// Includes originally from uWebSockets examples/helpers but copied and improved.
#include "uwebsockets_async_file_reader.h"
#include "uwebsockets_async_file_streamer.h"
#include "uwebsockets_middleware.h"

// WarHawkReborn includes
#include "forward_server.h"
#include "http_server.h"
#include "request_server.h"
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


void Usage( )
{
  std::cout << "Usage: warhawkreborn [ ( -h | --help ] [ ( -v | --version ) ] [ ( -p | --port ) <port> ] [ ( -r | --root) <root> ]" << std::endl;
}


// Main program
int main( int argc_, const char **argv_ )
{
  std::string program = argv_[ 0 ];

  int port = WARHAWK_HTTP_PORT;
  std::string root = ""; // When it is blank then the http server will add "./html" to the current directory.

  // Check to see whether we should parse any command-line arguments.
  if ( argc_ > 1 )
  {
    for ( int i = 1; i < argc_; ++i )
    {
      std::string option = argv_[ i ];

      if ( argc_ == 2 && ( option == "-h" || option == "--help" ) )
      {
        Usage( );
        return 0;
      }

      if ( option == "-p" || option == "--port" )
      {
        ++i;
        port = atoi( argv_[ i ] );
        continue;
      }

      if ( option == "-r" || option == "--root" )
      {
        ++i;
        root = argv_[ i ];
        continue;
      }

      // Check for -v or --version commandline argument.
      if ( argc_ == 2 && ( option == "-v" || option == "--version" ) )
      {
        std::cout << VersionString( ) << std::endl;

        return 0;
      }

      std::cerr << "Unknown option '" << option << "'." << std::endl;
      Usage( );
      return 1;
    }
  }

  if ( root == "" )
  {
    // Search upwards in the directory tree until we find the html directory.
    std::filesystem::path path = program;
    bool found = false;

    while ( path.string( ).length( ) > 0 && !found )
    {
      if ( std::filesystem::exists( path.string( ) + "/html" ) )
      {
        found = true;
        break;
      }

      path = path.parent_path( );
    }

    if ( !found )
    {
      // Check defaultRoot directory.
      const std::string defaultRoot = "/usr/share/warhawkreborn/html";

      if ( std::filesystem::exists( defaultRoot ) )
      {
        root = defaultRoot;
      }
      else
      {
        std::cerr << "Can't locate HTML directory." << std::endl;
        return 1;
      }
    }
    else
    {
      root = path.string( ) + "/html";
    }
  }

  std::cout << VersionString( ) << std::endl;

  std::cout << "Warhawk bridge booting..." << std::endl;

  try
  {
    // Set up to listen for UDP packets on standard WarHawk port.
    warhawk::net::udp_server udpServer( WARHAWK_UDP_PORT );

    // The Packet server watches for packets and distributes them to the
    // clients that register with it.
    Server packetServer( udpServer );

    std::thread packetServerThread( [&] ( )
    {
      std::cout << "Starting Packet Server..." << std::endl;
      packetServer.run( );
      std::cout << "Stopping Packet Server." << std::endl;
    } );

    // The ForwardServer watches for requests from the local network and
    // responds with a list of remote servers.
    ForwardServer forwardServer( &packetServer );

    // The SearchServer broadcasts a request for servers on the local network.
    // Any responses it receives are then sent on to the remote server that
    // publishes the list of available public servers.
    SearchServer searchServer( &packetServer );

    std::thread searchServerThread( [&] ( )
    {
      std::cout << "Starting Search Server..." << std::endl;
      searchServer.run( );
      std::cout << "SearchServer thread ended." << std::endl;
    } );

    // The RequestServer periodically queries the remote WarHawk Server List Server
    // and puts the resuling list of servers into the forwardServer and searchServer.
    RequestServer requestServer( forwardServer, packetServer, searchServer );

    std::thread requestServerThread( [&] ( )
    {
      std::cout << "Starting Request Server..." << std::endl;
      requestServer.run( );
      std::cout << "RequestServer thread ended." << std::endl;
    } );

    HttpServer httpServer( port, root, forwardServer );
    httpServer.run( );

    packetServerThread.join( );
    searchServerThread.join( );
    requestServerThread.join( );
  }
  catch ( const std::exception e_ )
  {
    std::cout << "Error: " << e_.what( ) << std::endl;
    return -1;
  }

  return 0;
}
