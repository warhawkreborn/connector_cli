#pragma once

//
// The ForwardServer watches for requests for servers from the local network and
// responds to valid requests with a list of remote servers.
//

#include <functional>
#include <iostream>

#include "message_handler.h"
#include "packet_processor.h"
#include "server_list.h"
#include "udp_network_socket.h"

class SearchServer;
class Network;


class ForwardServer : public MessageHandler
{
  public:

    ForwardServer( ServerList &, PacketProcessor &, Network & );
    ~ForwardServer( );
 
    void OnReceivePacket( sockaddr_storage client, std::vector< uint8_t > data ) override;

  protected:

  private:
    //
    // Methods
    //

    bool valid_packet( const std::vector< uint8_t > &data_ );

    //
    // Data
    //

    ServerList      &m_ServerList;
    PacketProcessor &m_PacketProcessor;
    Network         &m_Network;
};
