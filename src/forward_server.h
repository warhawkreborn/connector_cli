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


class Packet;
class Network;
class SearchServer;


class ForwardServer : public MessageHandler
{
  public:

    ForwardServer( ServerList &, PacketProcessor &, Network & );
    ~ForwardServer( );
 
    void OnReceivePacket( sockaddr_storage client, const Packet & ) override;

  protected:

  private:
    //
    // Methods
    //

    bool valid_packet( const Packet & );

    //
    // Data
    //

    ServerList      &m_ServerList;
    PacketProcessor &m_PacketProcessor;
    Network         &m_Network;
};
