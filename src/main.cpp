#include <iostream>
#include <mutex>
#include <thread>

#include "forward_server.h"
#include "net.h"
#include "picojson.h"
#include "search_server.h"
#include "server.h"
#include "server_entry.h"
#include "warhawk.h"
#include "webclient.h"


std::vector< ServerEntry > download_server_list( Server *server_ )
{
  auto req = warhawk::common::request::default_get( "https://warhawk.thalhammer.it/api/server/" );
  warhawk::common::webclient client;
  client.set_verbose( false );
  auto resp = client.execute( req );

  if ( resp.m_status_code != 200 )
  {
    throw std::runtime_error( "http request failed" );
  }

  picojson::value val;
  auto err = picojson::parse( val, resp.m_data );

  if ( !err.empty() )
  {
    throw std::runtime_error( "invalid json:" + err );
  }

  std::vector< ServerEntry > res;

  for ( auto &e : val.get< picojson::array >() )
  {
    try
    {
      auto ip = warhawk::net::udp_server::get_ip( e.get( "hostname" ).get< std::string >() );

      if ( e.get( "state" ).get< std::string >() != "online" )
      {
        continue;
      }

      ServerEntry entry;
      entry.m_name = e.get( "name" ).get< std::string >();
      entry.m_ping = static_cast< int >( e.get( "ping" ).get< int64_t >( ) );
      entry.m_frame = server_->hex2bin( e.get( "response" ).get< std::string >() );

      auto frame = (warhawk::net::server_info_response *) ( entry.m_frame.data() + 4 );

      memcpy( frame->m_ip1, ip.data(), ip.size() );
      memcpy( frame->m_ip2, ip.data(), ip.size() );

      res.push_back( entry );
    }
    catch ( const std::exception &e_ )
    {
      std::cout << "DownloadServerList: failed to parse server entry:" << e_.what() << std::endl;
    }
  }

  return res;
}

int main( int argc_, const char **argv_ )
{
  std::cout << "Warhawk bridge booting..." << std::endl;

  warhawk::net::udp_server udpServer( 10029 );

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
    auto list = download_server_list( &packetServer );
    forwardServer.set_entries( list );
    std::cout << "MainLoop: " << list.size() << " servers found" << std::endl;

    for ( auto &e : list )
    {
      std::cout << "MainLoop: " << e.m_name << " " << e.m_ping << "ms" << std::endl;
    }

    std::this_thread::sleep_for( std::chrono::seconds( 60 ) );
  }

  return 0;
}
