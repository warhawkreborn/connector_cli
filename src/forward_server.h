#ifndef FORWARD_SERVER_H
#define FORWARD_SERVER_H

#include <iostream>

#include "message_handler.h"
#include "net.h"
#include "server.h"
#include "server_entry.h"


class ForwardServer : public MessageHandler
{
  public:

    ForwardServer( Server * );
    ~ForwardServer( );
 
    void set_entries( std::vector< ServerEntry > e_ );

    void OnReceivePacket( struct sockaddr_in client, std::vector< uint8_t > data ) override;

  protected:

  private:
    //
    // Methods
    //

    bool valid_packet( const std::vector< uint8_t > &data_ );

    //
    // Data
    //

    std::mutex                  m_mutex;
    std::vector< ServerEntry >  m_entries;
    Server                     *m_server;
};

#endif // FORWARD_SERVER_H
