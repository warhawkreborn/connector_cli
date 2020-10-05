#pragma once

//
// This class contains the network socket and port for active connections.
// It allows the caller to set up a socket, bind it to a port, and then
// send and receive packets on that port/socket.
//

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


class Network;


namespace warhawk
{

namespace net
{

class UdpNetworkSocket
{
  public:

    UdpNetworkSocket( Network &, uint16_t port ); // Bind a port to the socket.
    ~UdpNetworkSocket();

    UdpNetworkSocket( const UdpNetworkSocket & ) = delete; // Don't allow copying of this class.
    UdpNetworkSocket &operator=( const UdpNetworkSocket & ) = delete; // Don't allow copying of this class.

    // Send a packet from this port.
    void send(    const sockaddr_storage &clientaddr, const std::vector< uint8_t > &data, const bool broadcast = false );

    // Receive a packet from this port.
    bool receive(       sockaddr_storage &clientaddr,       std::vector< uint8_t > &data );

    // Convert a host address string to binary format.
    static std::array< uint8_t, 4 > StringToIp( const std::string &host );
    // Convert a host address from binary to string format.
    static std::string              IpToString( const std::array< uint8_t, 4 > & ip );

    // Get the port associated with this socket.
    uint16_t GetPort( ) const;

  private:

    SOCKET   m_fd;
    uint16_t m_port;
    Network &m_Network;
};

} // End namespace net

} // End namespace warhawk
