#pragma once

//
// The client server is created by the proxy server when it detects packets from a new 
// client.  It then passes the packets back and forth between the client and the local
// server thereby performing the client function of the proxy server.
//

#include <memory>
#include <string>
#include <thread>

#include "message_handler.h"
#include "packet_processor.h"
#include "udp_network_socket.h"


class Network;
class ServerList;
class Packet;


class ClientServer : public MessageHandler
{
  public:

    ClientServer( const std::string &publicIpAddress,
                  const int &publicPort,
                  ServerList &,
                  Network &,
                  PacketProcessor &proxyToClientPacketProcessor );
    ~ClientServer( );

    std::string GetPublicIp( );

    void SendPacket( const Packet & ); // Send Packet to WarHawk Server

  protected:

  private:

    //
    // Methods
    //

    void OnReceivePacket( const Packet &packet_ ); // Receive Packet from WarHawkServer

    //
    // Data
    //

    std::string      m_PublicIpAddress;
    const int        m_PublicPort;
    ServerList      &m_ServerList;
    Network         &m_Network;
    PacketProcessor                                  &m_ProxyToClientPacketProcessor;
    std::unique_ptr< warhawk::net::UdpNetworkSocket > m_ProxyToServerSocket;
    std::unique_ptr< PacketProcessor >                m_ProxyToServerPacketProcessor;    
};


using ClientServerPtr = std::unique_ptr< ClientServer >;
