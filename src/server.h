#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <map>

#include "message_handler.h"
#include "net.h"
#include "server_entry.h"


class MessageHandler;


class Server
{
  public:

    Server( warhawk::net::udp_server &udpServer_ );
    virtual ~Server();

    void run( );

    void send(    const sockaddr_storage &clientaddr, const std::vector< uint8_t > &data, const bool broadcast = false );
    bool receive(       sockaddr_storage &clientaddr, std::vector< uint8_t > &data );

    bool valid_packet( const std::vector< uint8_t > &data_ );

    warhawk::net::udp_server &GetServer( );

    void Register(   MessageHandler * );
    void Unregister( MessageHandler * );

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
};

#endif // SERVER_H
