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
#include "server_list.h"
#include "udp_network_socket.h"


class Network;
class Packet;
class PacketProcessor;


class ProxyServer : public MessageHandler
{
  public:

    ProxyServer( ServerList &, PacketProcessor &, Network & );
    ~ProxyServer( );
 
    void OnReceivePacket( const Packet & ) override;

  protected:

  private:

    //
    // Methods
    //

    void OnHandleServerInfoRequest(  const Packet & );
    void OnHandleServerInfoResponse( const Packet & );

    void OnHandleGameClientToServer( const Packet & );
    void OnHandleGameServerToClient( const Packet & );

    //
    // Data
    //

    ServerList      &m_ServerList;
    PacketProcessor &m_PacketProcessor;
    Network         &m_Network;
    bool             m_ProxyMode       = false;
    bool             m_ReplyingToQuery = false;
    std::string      m_ServerListServer; // warhawk.thalhamer.it.
    uint16_t         m_LastServerListServerPort; // Port we last received packet from ServerListServer on.

    typedef struct
    {
      Packet m_PacketFromClient;
      Packet m_PacketFromServer;
    } ClientData;
    
    using ClientIpList = std::map< std::string, ClientData >;
    ClientIpList     m_ClientIpList;
};
