#pragma once

//
// This class watches for packets and distributes them to the
// clients that register with it.
//

#include <iostream>
#include <map>
#include <thread>

#include "message_handler.h"
#include "udp_server.h"
#include "server_entry.h"


class MessageHandler;


class Server
{
  public:

    Server( warhawk::net::udp_server &udpServer_ );
    virtual ~Server();

    // Run the Server process.
    void run( );

    // Send packets.
    void send(    const sockaddr_storage &clientaddr, const std::vector< uint8_t > &data, const bool broadcast = false );

    // Receive packets.
    bool receive(       sockaddr_storage &clientaddr, std::vector< uint8_t > &data );

    // Check for valid packet.
    bool valid_packet( const std::vector< uint8_t > &data_ );

    // Return the associated UdpServer.
    warhawk::net::udp_server &GetServer( );

    // Register a message handler.
    void Register(   MessageHandler * );

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

    warhawk::net::udp_server &m_server;
 
    std::mutex      m_mutex; // Protect m_MessageHandlers.
    using MessageHandlers = std::map< MessageHandler *, MessageHandler * >;
    MessageHandlers m_MessageHandlers;

    bool        m_Done = false;

    // Make sure this is always last so that the thread destructs (joins) first.
    std::thread m_Thread;
};
