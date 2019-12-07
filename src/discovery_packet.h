#ifndef DISCOVERY_PACKET_H
#define DISCOVERY_PACKET_H

#include <cstdint>
#include <vector>


namespace warhawk
{

class DiscoveryPacket
{
  public:

    DiscoveryPacket( std::vector< uint8_t > data );

    std::string GetName( );

    std::string GetGameMode( );

    std::string GetMap( );

    std::string GetMapName( );

    int GetMapSize( );

    int GetMaxPlayers( );

    int GetCurrentPlayers( );

    int GetMinPlayers( );

    int GetTimeElapsed( );

    int GetTimeLimit( );

    int GetStartWaitTime( );

    int GetSpawnWaitTime( );

    int GetRoundsPlayed( );

    int GetPointLimit( );

    int GetCurrentPoints( );

    void setPort( int i );

    // TODO - Some rep of IPv4 addr = void SetIP( int addr );

    // TODO - Some rep of IPv4 addr = void SetIP( int addr );

    void SetIP( std::vector< uint8_t > addrBytes );

    std::vector< uint8_t > GetBytes( );

  private:

    std::vector< uint8_t > m_data;
};

} // End namespace warhawk

#endif // DISCOVERY_PACKET_H
