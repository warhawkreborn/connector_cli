#ifndef WARHAWK_H
#define WARHAWK_H

#pragma once

#include <cstdint>
#include <vector>

const int         WARHAWK_UDP_PORT = 10029;
const std::string WARHAWK_API_BASE = "https://warhawk.thalhammer.it/api/";


namespace warhawk
{

namespace net
{

struct server_info_response
{
  uint8_t  m_unknown_1[ 108 ];
  uint8_t  m_ip1[ 4 ];
  uint8_t  m_unknown_2[ 60 ];
  uint8_t  m_ip2[ 4 ];
  uint8_t  m_servername[ 32 ];
  uint8_t  m_mapname[ 24 ];
  uint8_t  m_unknown_3;
  uint8_t  m_gamemode;
  uint8_t  m_unknown_4;
  uint8_t  m_max_players;
  uint8_t  m_unknown_5[ 2 ];
  uint8_t  m_current_players;
  uint8_t  m_unknown_6[ 8 ];
  uint8_t  m_time_eclapsed;
  uint8_t  m_unknown_7[ 4 ];
  uint8_t  m_mapsize[ 16 ];
  uint16_t m_point_limit; // Note: Big endian
  uint16_t m_point_current; // Note: Big endian
  uint8_t  m_unknown_8[ 3 ];
  uint8_t  m_time_limit;
  uint8_t  m_min_players;
  uint8_t  m_unknown_9;
  uint8_t  m_start_wait;
  uint8_t  m_spawn_wait;
  uint8_t  m_flags;
  uint8_t  m_min_rank;
  uint8_t  m_max_rank;
  uint8_t  m_unknown_10[ 28 ];
  uint8_t  m_rounds_played;
  uint8_t  m_unknown_11[ 56 ];
};

static_assert( sizeof( server_info_response ) == 368, "Size missmatch, check compiler" );

template< typename T >
inline std::vector< uint8_t > build_packet( uint8_t t1_, uint8_t t2_, const T& data_ )
{
  std::vector< uint8_t > res;
  res.resize( sizeof( data_ ) + 4 );
  res[0] = t1_;
  res[1] = t2_;
  res[2] = sizeof( data_ ) & 0xff;
  res[3] = ( sizeof( data_ ) >> 8 ) & 0xff;
  memcpy( res.data( ), &data_, sizeof( data_ ) );
  return res;
}

} // End namespace net

} // End namespace warhawk

#endif // WARHAWK_H
