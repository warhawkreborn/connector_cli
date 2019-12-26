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

#include "forward_server.h"
#include "http_server.h"


HttpServer::HttpServer( const int port_, const std::string rootDirectory_, ForwardServer &forwardServer_ )
  : m_Port( port_ )
  , m_RootDirectory( rootDirectory_ )
  , m_ForwardServer( forwardServer_ )
{
}


HttpServer::~HttpServer( )
{
}


// Main program
void HttpServer::run( )
{
  if ( m_RootDirectory == "" )
  {
    char *ptr = getcwd( nullptr, 0 );
    if ( ptr == nullptr )
    {
      std::cout << "Can't get current working directory." << std::endl;
    }
    else
    {
      std::string currentDir = ptr;
      m_RootDirectory = currentDir + "/html";
    }
  }

  try
  {
    AsyncFileStreamer asyncFileStreamer( m_RootDirectory );

    while ( true )
    {
      try
      {
        uWS::App( )
          // /api/servers endpoint.
          .get( "/api/servers", [ this ] ( auto *res_, auto *req_ )
          {
            std::string html = OnGetApiServers( );

            res_->end( html );
          } )
          // Serve files.
          .get( "/*", [ &asyncFileStreamer ]( auto *res_, auto *req_ )
          {
            res_->onAborted( [ res_ ] ( )
            {
              std::cout << "Get method aborted on error." << std::endl;
            } );

            serveFile( res_, req_ );
            std::string_view svUrl = req_->getUrl( );
            std::string url( svUrl.data( ), svUrl.size( ) );

            if ( url == "/" )
            {
              url = "/index.html";
            }

            try
            {
              asyncFileStreamer.streamFile( res_, url );
            }
            catch( const std::exception &e_ )
            {
              std::stringstream ss;
              ss << "HTTP Server error: " << e_.what( ) << std::endl;
              res_->end( ss.str( ) );
            }
          } )
          .listen( m_Port, [ this ]( auto *token_ )
          {
            if ( token_ )
            {
              OnListen( );
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
  catch ( const std::exception &e_ )
  {
    std::cout << "HTTP Server error: " << e_.what( ) << std::endl;
  }
}


void HttpServer::OnListen( )
{
  std::cout << "HTTP Server on port " << m_Port << ", serving directory '" << m_RootDirectory << "'." << std::endl;
}


std::string HttpServer::OnGetApiServers( )
{
  std::stringstream html;

  html << "<html>" << std::endl;
  html << "<head>" << std::endl;
  html << "  <title>WarHawkReborn</title>" << std::endl;
  html << "</head>" << std::endl;
  html << "<body bgcolor='#000000'>" << std::endl;

  html << "<center>" << std::endl;

  html << "<img src='/images/warhawk.png'>" << std::endl;

  html << "<p>" << std::endl;

  html << "<h1 style='color: #00ff00;'>Servers</h1>" << std::endl;

  html << "</p>" << std::endl;

  html << "<table>" << std::endl;
  html << "  <tr>" << std::endl;
  html << "    <th style='color: #00ff00;'>Name</th>" << std::endl;
  html << "    <th style='color: #00ff00;'>IP Address</th>" << std::endl;
  html << "  </tr>" << std::endl;

  m_ForwardServer.ForEachServer( [&html]( auto entry_ ) {
    html << "  <tr>" << std::endl;
    html << "    <td style='color: #00ff00;'>" << entry_.m_name << "</td>" << std::endl;
    html << "    <td style='color: #00ff00;'>" << entry_.m_ip << "</td>" << std::endl;
    html << "  </tr>" << std::endl;
  } );

  html << "</table>" << std::endl;

  html << "</center>" << std::endl;

  html << "</body>" << std::endl;
  html << "</html>" << std::endl;

  return html.str( );
}
