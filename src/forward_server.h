#pragma once

//
// The ForwardServer watches for requests for servers from the local network and
// responds to valid requests with a list of remote servers.
//

#include <functional>
#include <iostream>

#include "message_handler.h"
#include "packet_server.h"
#include "server_list.h"
#include "udp_network_socket.h"

class SearchServer;
class Network;


class ForwardServer : public MessageHandler
{
  public:

    ForwardServer( ServerList &, PacketServer &, Network & );
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

    ServerList   &m_ServerList;
    PacketServer &m_PacketServer;
    Network      &m_Network;
};
