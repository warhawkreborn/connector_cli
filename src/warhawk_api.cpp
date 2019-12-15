#include "net.h"
#include "picojson.h"
#include "server.h"
#include "server_entry.h"
#include "warhawk.h"
#include "warhawk_api.h"
#include "webclient.h"


namespace warhawk
{

std::vector< ServerEntry > API::DownloadServerList( Server *server_ )
{
  auto req = warhawk::common::request::default_get( WARHAWK_API_BASE + "server/" );
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
      auto ip = warhawk::net::udp_server::StringToIp( e.get( "hostname" ).get< std::string >( ) );

      if ( e.get( "state" ).get< std::string >( ) != "online" )
      {
        continue;
      }

      ServerEntry entry;
      entry.m_name = e.get( "name" ).get< std::string >( );
      entry.m_ping = static_cast< int >( e.get( "ping" ).get< int64_t >( ) );
      entry.m_frame = server_->hex2bin( e.get( "response" ).get< std::string >( ) );
      // NOTE: Converting hostname to array and back might seems redundant, but its not,
      // as hostname can be a domain, and ip is guaranteed to be a ipv4 ip.
      entry.m_ip = warhawk::net::udp_server::IpToString( ip );

      if ( entry.m_frame.size() < 4 || (entry.m_frame.size() - 4) < sizeof(warhawk::net::server_info_response) )
      {
        // Just to be safe....
        continue;
      }

      auto frame = reinterpret_cast<warhawk::net::server_info_response *>( entry.m_frame.data() + 4 );

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


API::ForwardingResponse API::CheckForwarding( )
{
  auto req = warhawk::common::request::default_get( WARHAWK_API_BASE + "server/checkForwarding" );
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

  std::string ip;
  std::string state;

  try
  {
    ip    = val.get( "ip"   ).get< std::string >( );
    state = val.get( "info" ).get( "state" ).get< std::string >( );
  }
  catch ( const std::exception &e_ )
  {
    std::cout << "DownloadServerList: failed to parse server entry:" << e_.what( ) << std::endl;
    ip = "";
    state = "";
  }

  ForwardingResponse response;
  response.m_ip    = ip;
  response.m_state = state;

  return response;
}


std::string API::AddHost( std::string hostname_, std::string uniqueId_, bool persistent_ )
{
  std::cout << "AddHost: Name = " << hostname_ << ", uniqueId = " << uniqueId_ << ", persistent = " << persistent_ << std::endl;

  picojson::object jsonObject;
  jsonObject[ "hostname"   ] = picojson::value( std::string( hostname_ ) );
  if ( !uniqueId_.empty( ) )
  {
    jsonObject[ "fcm_id"     ] = picojson::value( std::string( uniqueId_ ) );
  }
  jsonObject[ "persistent" ] = picojson::value( persistent_ );
  std::string jsonString = picojson::value( jsonObject ).serialize( );

  auto req = warhawk::common::request::default_post( WARHAWK_API_BASE + "server/", jsonString );
  warhawk::common::webclient client;
  client.set_verbose( false );
  auto resp = client.execute( req );

  if ( resp.m_status_code != 200 )
  {
    throw std::runtime_error( "http request failed" );
  }

  return "ok";
}

} // namespace warhawk
