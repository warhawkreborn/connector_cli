#pragma once

//
// The WarHawk DiscoveryPacket contains information about the current WarHawk server
// including current map, number of players, and other information.
//

#include <cstdint>
#include <string>
#include <vector>


#include "packet.h"


namespace warhawk
{

class DiscoveryPacket : public Packet
{
  public:

    DiscoveryPacket( );

    DiscoveryPacket( Packet );

    std::string GetName( ) const;

    std::string GetGameMode( ) const;

    std::string GetMap( ) const;

    std::string GetMapName( ) const;

    int GetMapSize( ) const;

    int GetMaxPlayers( ) const;

    int GetCurrentPlayers( ) const;

    int GetMinPlayers( ) const;

    int GetTimeElapsed( ) const;

    int GetTimeLimit( ) const;

    int GetStartWaitTime( ) const;

    int GetSpawnWaitTime( ) const;

    int GetRoundsPlayed( ) const;

    int GetPointLimit( ) const;

    int GetCurrentPoints( ) const;

    void setPort( int i );

    // TODO - Some rep of IPv4 addr = void SetIP( int addr );

    // TODO - Some rep of IPv4 addr = void SetIP( int addr );

    void SetIP( std::vector< uint8_t > addrBytes );

    std::vector< uint8_t > GetBytes( );

  private:

};

} // End namespace warhawk
