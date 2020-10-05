#pragma once

//
// This class watches for packets and distributes them to the
// clients that register with it.
//

#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "message_handler.h"
#include "udp_network_socket.h"
#include "server_list.h"


class MessageHandler;
class Network;


class PacketProcessor
{
  public:

    PacketProcessor( const std::string &name, warhawk::net::UdpNetworkSocket &, Network & );
    virtual ~PacketProcessor( );

    // Run the Server process.
    void run( );

    // Send packets.
    void send(    const sockaddr_storage &clientaddr, const std::vector< uint8_t > &data, const bool broadcast = false );

    // Receive packets.
    bool receive(       sockaddr_storage &clientaddr, std::vector< uint8_t > &data );

    // Return the associated UdpServer.
    warhawk::net::UdpNetworkSocket &GetServer();

    // Register a message handler.
    void Register(   MessageHandler *, int messageMask );

    // Unregister a message handler.
    void Unregister( MessageHandler * );

    // Convert a HEX-formatted string to binary.
    std::vector< uint8_t > hex2bin( const std::string & );

  protected:

  private:
    //
    // Methods
    //

    //
    // Data
    //

    std::string                     m_Name;
    warhawk::net::UdpNetworkSocket &m_UdpNetworkSocket;
    Network                        &m_Network;

    std::mutex      m_mutex; // Protect m_MessageHandlers.
    using MessageHandlers = std::map< MessageHandler *, int >;
    MessageHandlers m_MessageHandlers;

    bool        m_Done = false;

    // Make sure this is always last so that the thread destructs (joins) first.
    std::thread m_Thread;
};
