#ifndef NET_H
#define NET_H

#pragma once

#ifdef WIN32
#include <WS2tcpip.h>
#include <Windows.h>
#include <winsock2.h>
#endif

#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

#ifndef WIN32
#include <netinet/in.h>
#define SOCKET int
#endif


const int WARHAWK_UDP_PORT = 10029;


class Network;


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

    void send(    const sockaddr_storage &clientaddr, const std::vector< uint8_t > &data, const bool broadcast = false );
    bool receive(       sockaddr_storage &clientaddr, std::vector< uint8_t > &data );

    static std::array< uint8_t, 4 > get_ip( const std::string &host );

    uint16_t GetPort( ) const;

  private:

    SOCKET   m_fd;
    uint16_t m_port;
    Network *m_Network;
};

} // End namespace net

} // End namespace warhawk

#endif // NET_H
