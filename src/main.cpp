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

    while ( path.string( ).length( ) > 1 && !found )
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
#ifdef __APPLE__
      const std::string defaultRoot = "/usr/local/share/warhawkreborn/html";
#else
      const std::string defaultRoot = "/usr/share/warhawkreborn/html";
#endif

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


    // The ForwardServer watches for requests from the local network and
    // responds with a list of remote servers.
    ForwardServer forwardServer( &packetServer );

    // The SearchServer broadcasts a request for servers on the local network.
    // Any responses it receives are then sent on to the remote server that
    // publishes the list of available public servers.
    SearchServer searchServer( &packetServer );

    // The RequestServer periodically queries the remote WarHawk Server List Server
    // and puts the resuling list of servers into the forwardServer and searchServer.
    RequestServer requestServer( forwardServer, packetServer, searchServer );

#ifndef TEST_SHUTDOWN
    // Run the HTTP server in the main thread.
    HttpServer httpServer( port, root, forwardServer, searchServer );
    httpServer.run( );
#endif

    // Servers and threads will destruct here as the variables go out of scope.
  }
  catch ( const std::exception e_ )
  {
    std::cout << "FATAL ERROR - Shutting down. Error: " << e_.what( ) << std::endl;
    return -1;
  }

  std::cout << "WarHawk Reborn server shutting down properly." << std::endl;

  return 0;
}
