#ifndef FORWARD_SERVER_H
#define FORWARD_SERVER_H

#include <iostream>

#include "net.h"
#include "server.h"
#include "server_entry.h"


class ForwardServer : public Server
{
  public:

    ForwardServer( warhawk::net::udp_server &udpServer_ );
 
    void set_entries( std::vector< ServerEntry > e_ );

    void run( ) override;

  protected:

  private:
    //
    // Methods
    //

    bool valid_packet( const std::vector< uint8_t > &data_ );

    //
    // Data
    //

    std::mutex m_mtx;
    std::vector< ServerEntry > m_entries;
};

#endif // FORWARD_SERVER_H
