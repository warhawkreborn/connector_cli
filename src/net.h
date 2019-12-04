#ifndef NET_H
#define NET_H_

#pragma once

#ifdef WIN32
#include <Windows.h>
#endif

#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

#ifndef WIN32
#include <netinet/in.h>
#define SOCKET int
#endif


namespace warhawk
{

namespace net
{

class udp_server
{
  public:

    udp_server( uint16_t port );
    ~udp_server( );

    udp_server( const udp_server & ) = delete;
    udp_server &operator=( const udp_server & ) = delete;

    void send(    struct sockaddr_in &clientaddr, const std::vector< uint8_t > &data );
    bool receive( struct sockaddr_in &clientaddr, std::vector< uint8_t > &data );

    static std::array< uint8_t, 4 > get_ip( const std::string &host );

  private:

    SOCKET m_fd;
    std::mutex m_mtx;
};

} // End namespace net

} // End namespace warhawk

#endif // NET_H
