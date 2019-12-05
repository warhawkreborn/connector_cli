#ifndef SERVER_H
#define SERVER_H

#include <iostream>

#include "net.h"
#include "server_entry.h"

class Server
{
  public:

    Server( warhawk::net::udp_server &udpServer_ );
    virtual ~Server();

    virtual void run() = 0;

    std::mutex &GetMutex();

    bool valid_packet( const std::vector< uint8_t > &data_ );

    warhawk::net::udp_server &GetServer( );

 protected :

    private :
    //
    // Methods
    //

    //
    // Data
    //

    static std::mutex s_mtx;
    warhawk::net::udp_server &m_server;
};

#endif // SERVER_H
