#include <stdexcept>

#include "discovery_packet.h"


namespace warhawk
{

DiscoveryPacket::DiscoveryPacket( )
  : Packet( )
{
}

DiscoveryPacket::DiscoveryPacket( Packet packet_ )
  : Packet( packet_ )
{
}

std::string DiscoveryPacket::GetName( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return "";
  }

  int i = 0;

  for ( i = 0; i < 32; i++ )
  {
    if ( GetData( )[ 180 + i ] == 0 )
    {
      break;
    }
  }

  std::string name = std::string( (char *) &GetData( )[ 180 ], i );
  return name;
}


std::string DiscoveryPacket::GetGameMode( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return "";
  }

  switch( GetData( )[ 237 ] )
  {
    case 0:  return "DM";
    case 1:  return "TDM";
    case 2:  return "Ctf";
    case 3:  return "Zones";
    case 4:  return "Hero";
    case 5:  return "Collection";
    default: return "Unknown gamemode";
  }
}

std::string DiscoveryPacket::GetMap( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return "";
  }

  int i = 0;

  for ( i = 0; i < 24; i++ )
  {
    if ( GetData( )[ 212 + i ] == 0 )
    {
      break;
    }
  }
  
  std::string map = std::string( (char *) &GetData( )[ 212 ], i );
  return map;
}


std::string DiscoveryPacket::GetMapName( ) const
{
  std::string map = GetMap( );

  if ( map == "multi01" ) return "Eucadia";
  if ( map == "multi02" ) return "Island Outpost";
  if ( map == "multi03" ) return "The Badlands";
  if ( map == "multi05" ) return "Destroyed Capitol";
  if ( map == "multi06" ) return "Omega Factory";
  if ( map == "multi07" ) return "Archipelago";
  if ( map == "multi08" ) return "Vaporfield Glacier";
  if ( map == "multi09" ) return "Tau Crater";
  return "Unknown";
}

int DiscoveryPacket::GetMapSize( ) const
{
  int i = 0;
  int start = -1;

  for ( i = 0; i < 16; i++ )
  {
    if ( GetData( )[ 256 + i ] == 0 )
    {
      break;
    }

    if ( start == -1 && GetData( )[ 256 + i ] < 58 && GetData( )[ 256 + i ] > 47 )
    {
      start = i;
    }
  }

  std::string mode = std::string( (char *) &GetData( ), 256, i );
  return GetData( )[ 336 ];
}


int DiscoveryPacket::GetMaxPlayers( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 239 ];
}

int DiscoveryPacket::GetCurrentPlayers( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 242 ];
}


int DiscoveryPacket::GetMinPlayers( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 280 ];
}


int DiscoveryPacket::GetTimeElapsed( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 251 ];
}


int DiscoveryPacket::GetTimeLimit( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 279 ];
}


int DiscoveryPacket::GetStartWaitTime( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 282 ];
}


int DiscoveryPacket::GetSpawnWaitTime( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 283 ];
}


int DiscoveryPacket::GetRoundsPlayed( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 315 ] & 0xff;
}


int DiscoveryPacket::GetPointLimit( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 272 ] << 8 | GetData( )[ 273 ];
}


int DiscoveryPacket::GetCurrentPoints( ) const
{
  if ( GetData( ).size( ) == 0 )
  {
    return 0;
  }

  return GetData( )[ 274 ] << 8 | GetData( )[ 275 ];
}

#ifdef FUTURE
void DiscoveryPacket::SetPort( int i_ )
{
  int lsb = i_ & 0xff;
  int msb = ( i_ & 0xff00 ) >> 8;
  GetData( )[ 120 ] = (uint8_t) lsb;
  GetData( )[ 121 ] = (uint8_t) msb;
}


void DiscoveryPacket::SetIP( Inet4Address addr)
{
  byte[] addrBytes = addr.getAddress();
  this.setIP( addrBytes );
}

public void DiscoveryPacket::SetIP( InetAddress addr)
{
  if ( addr instanceof Inet4Address )
  {
    setIP( (Inet4Address) addr );
  }
  else
  {
    throw new IllegalArgumentException( "Only ipv4 addresses are supported" );
  }
}
#endif


void DiscoveryPacket::SetIP( std::vector< uint8_t > addrBytes_ )
{
  if ( addrBytes_.size( ) == 0 || addrBytes_.size( ) != 4 )
  {
    throw std::runtime_error( "Invalid address bytes" );
  }

  GetData( )[ 112 ] = addrBytes_[ 0 ];
  GetData( )[ 113 ] = addrBytes_[ 1 ];
  GetData( )[ 114 ] = addrBytes_[ 2 ];
  GetData( )[ 115 ] = addrBytes_[ 3 ];

  GetData( )[ 176 ] = addrBytes_[ 0 ];
  GetData( )[ 177 ] = addrBytes_[ 1 ];
  GetData( )[ 178 ] = addrBytes_[ 2 ];
  GetData( )[ 179 ] = addrBytes_[ 3 ];
}

#ifdef FUTURE
public byte[] DiscoveryPacket::getBytes() {
    return data;
}
#endif

} // namespace warhawk
