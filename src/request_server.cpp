#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "forward_server.h"
#include "request_server.h"
#include "search_server.h"
#include "server.h"
#include "server_entry.h"
#include "warhawk_api.h"


RequestServer::RequestServer( ForwardServer &forwardServer_, Server &packetServer_, SearchServer &searchServer_ )
  : m_ForwardServer( forwardServer_ )
  , m_PacketServer(  packetServer_  )
  , m_SearchServer(  searchServer_  )
  , m_Thread( [&] ( ) { run( ); } )

{
}


RequestServer::~RequestServer( )
{
  m_Done = true;
  m_Thread.join( );
}


void RequestServer::run( )
{
  std::cout << "Starting Request Server..."<< std::endl;

  // This loop periodically queries the server that holds a list of available
  // public servers and updates that list into the SearchServer and ForwardServer.
  while ( !m_Done )
  {
    std::cout << "RequestServer: Updating server list" << std::endl;
    std::vector< ServerEntry > list;
    try
    {
      list = warhawk::API::DownloadServerList( &m_PacketServer );
    }
    catch ( const std::exception &e_ )
    {
      std::cout << "RequestServer: Error = " << e_.what() << std::endl;
      std::cout << "RequestServer: Check network connection." << std::endl;
    }

    if ( list.size() > 0 )
    {
      m_ForwardServer.SetEntries( list );
      m_SearchServer.SetEntries( list );

      std::cout << "RequestServer: " << list.size() << " servers found" << std::endl;

      // Print out the list.
      for ( auto &e : list )
      {
        std::cout << "RequestServer: " << e.m_name << " " << e.m_ping << "ms" << std::endl;
      }
    }

    std::this_thread::sleep_for( std::chrono::seconds( 60 ) );
  }

  std::cout << "RequestServer thread ended." << std::endl;
}
