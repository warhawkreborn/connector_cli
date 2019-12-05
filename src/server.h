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

    void send( struct sockaddr_in &clientaddr, const std::vector< uint8_t > &data );
    bool receive( struct sockaddr_in &clientaddr, std::vector< uint8_t > &data );

    std::mutex &GetMutex();

    bool valid_packet( const std::vector< uint8_t > &data_ );

    warhawk::net::udp_server &GetServer( );

    void Register(   MessageHandler * );
    void Unregister( MessageHandler * );

  protected:

  private:
    //
    // Methods
    //

    //
    // Data
    //

    static std::mutex s_mutex;
    warhawk::net::udp_server &m_server;
 
    using MessageHandlers = std::map< MessageHandler *, MessageHandler * >;
    MessageHandlers m_MessageHandlers;
};

#endif // SERVER_H
