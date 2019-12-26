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

#include "http_server.h"


HttpServer::HttpServer( const int port_, const std::string rootDirectory_ )
  : m_Port( port_ )
  , m_RootDirectory( rootDirectory_ )
{
}


HttpServer::~HttpServer( )
{
}


// Main program
void HttpServer::run( )
{
  int port = m_Port;
  std::string root = m_RootDirectory;

  AsyncFileStreamer asyncFileStreamer( m_RootDirectory );

  while ( true )
  {
    try
    {
      uWS::App( )
        .get( "/*", [ &asyncFileStreamer ]( auto *res, auto *req )
        {
          res->onAborted( [res] ( )
          {
            std::cout << "Get method aborted on error." << std::endl;
          } );

          serveFile( res, req );
          std::string_view svUrl = req->getUrl( );
          std::string url( svUrl.data( ), svUrl.size( ) );
          url = url.substr( 1 ); // Skip past first '/'.

          if ( url == "" )
          {
            url = "index.html";
          }

          try
          {
            asyncFileStreamer.streamFile( res, url );
          }
          catch( const std::exception &e_ )
          {
            std::stringstream ss;
            ss << "HTTP Server error: " << e_.what( ) << std::endl;
            res->end( ss.str( ) );
          }
        } )
        .listen( port, [ port, root ]( auto *token )
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
    catch ( const std::exception e_ )
    {
      std::cout << "Error: " << e_.what() << std::endl;
    }
  }
}
