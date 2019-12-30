#pragma once

//
// The ProxyServer watches for game packets coming from the remote server or
// the local PS3s and passes the packets through so that WarHawkReborn can
// act as an intermediary or "proxy" server.
// This requires the port forwarding to forward to the WarHawkReborn server
// instead of the local PS3 running WarHawk.
//

#include <functional>
#include <iostream>

#include "message_handler.h"
#include "packet_server.h"
#include "server_list.h"
#include "udp_server.h"

class Network;


class ProxyServer : public MessageHandler
{
  public:

    ProxyServer( ServerList &, PacketServer &, Network & );
    ~ProxyServer( );
 
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
    bool          m_ProxyMode = false;
    std::string   m_ServerListServer; // warhawk.thalhamer.it.
    std::vector < std::string > m_ClientIpList;
};
