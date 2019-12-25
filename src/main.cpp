//
// This program acts as an intermediary which does several things
// 1. Gets a list of current remote warhawk servers from https://warhawk.thalhammer.it/api/ URL.
// 2. Broadcasts that list of servers to a warhawk client when it requests the list of servers.
// 3. Watches for warhawk local warhawk servers on the network and sends those so the remote server.
//

// System includes
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
#pragma GCC diagnostic ignored "-w"
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

    const int port = 8080;
    const std::string root = "./";

    AsyncFileStreamer asyncFileStreamer( root );

    while ( true )
    {
      uWS::App( )
        .get( "/*", [&asyncFileStreamer]( auto *res, auto *req )
        {
          res->onAborted( [res] ( )
          {
            std::cout << "Get method aborted on error." << std::endl;
          } );

          serveFile( res, req );
          std::string_view svUrl = req->getUrl( );
          std::string url( svUrl.data( ), svUrl.size( ) );
          url = url.substr( 1 ); // Skip past first '/'.
          try
          {
            asyncFileStreamer.streamFile( res, url );
          }
          catch( const std::exception & )
          {
            std::cout << "HTTP Server can't find file '" << url << "'." << std::endl;
            res->end( "Can't find requested file.\r\n" );
          }
        } )
        .listen( port, [port, root]( auto *token )
        {
          if ( token )
          {
            char *ptr = getcwd( nullptr, 0 );
            if ( ptr == nullptr )
            {
              std::cout << "Can't get current working directory." << std::endl;
            }
            else
            {
              std::cout << "HTTP Server on port " << port << ", serving directory '" << ptr << "'." << std::endl;
            }
          }
        } )
        .run( );
    }

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
