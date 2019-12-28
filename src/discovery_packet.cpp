#include <stdexcept>

#include "discovery_packet.h"


namespace warhawk
{

DiscoveryPacket::DiscoveryPacket( std::vector< uint8_t > data_ )
  : m_data( data_ )
{
}

std::string DiscoveryPacket::GetName( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return "";
  }

  int i = 0;

  for ( i = 0; i < 32; i++ )
  {
    if ( m_data[ 180 + i ] == 0 )
    {
      break;
    }
  }

  std::string name = std::string( (char *) &m_data[ 180 ], i );
  return name;
}


std::string DiscoveryPacket::GetGameMode( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return "";
  }

  switch( m_data[ 237 ] )
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
  if ( m_data.size( ) == 0 )
  {
    return "";
  }

  int i = 0;

  for ( i=0; i < 24; i++ )
  {
    if ( m_data[ 212 + i ] == 0 )
    {
      break;
    }
  }
  
  std::string map = std::string( (char *) &m_data[ 212 ], i );
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
    if ( m_data[ 256 + i ] == 0 )
    {
      break;
    }

    if ( start == -1 && m_data[ 256 + i ] < 58 && m_data[ 256 + i ] > 47 )
    {
      start = i;
    }
  }

  std::string mode = std::string( (char *) &m_data, 256, i);
  return m_data[ 336 ];
}


int DiscoveryPacket::GetMaxPlayers( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 239 ];
}

int DiscoveryPacket::GetCurrentPlayers( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 242 ];
}


int DiscoveryPacket::GetMinPlayers( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 280 ];
}


int DiscoveryPacket::GetTimeElapsed( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 251 ];
}


int DiscoveryPacket::GetTimeLimit( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 279 ];
}


int DiscoveryPacket::GetStartWaitTime( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 282 ];
}


int DiscoveryPacket::GetSpawnWaitTime( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 283 ];
}


int DiscoveryPacket::GetRoundsPlayed( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 315 ] & 0xff;
}


int DiscoveryPacket::GetPointLimit( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 272 ] << 8 | m_data[ 273 ];
}


int DiscoveryPacket::GetCurrentPoints( ) const
{
  if ( m_data.size( ) == 0 )
  {
    return 0;
  }

  return m_data[ 274 ] << 8 | m_data[ 275 ];
}

#ifdef FUTURE
void DiscoveryPacket::SetPort( int i_ )
{
  int lsb = i_ & 0xff;
  int msb = ( i_ & 0xff00 ) >> 8;
  m_data[ 120 ] = (uint8_t) lsb;
  m_data[ 121 ] = (uint8_t) msb;
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

  m_data[ 112] = addrBytes_[ 0 ];
  m_data[ 113] = addrBytes_[ 1 ];
  m_data[ 114] = addrBytes_[ 2 ];
  m_data[ 115] = addrBytes_[ 3 ];

  m_data[ 176] = addrBytes_[ 0 ];
  m_data[ 177] = addrBytes_[ 1 ];
  m_data[ 178] = addrBytes_[ 2 ];
  m_data[ 179] = addrBytes_[ 3 ];
}

#ifdef FUTURE
public byte[] DiscoveryPacket::getBytes() {
    return data;
}
#endif

} // namespace warhawk
