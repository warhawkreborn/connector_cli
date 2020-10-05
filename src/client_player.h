#pragma once

//
// The ClientPlayer is created by the proxy server when it detects packets from a new 
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


class ClientPlayer : public MessageHandler
{
  public:

    ClientPlayer( const std::string &name,
                  const std::string &publicIpAddress,
                  const int &publicPort,
                  ServerList &,
                  Network &,
                  PacketProcessor &proxyToClientPacketProcessor );
    ~ClientPlayer( );

    std::string GetName( );

    std::string GetPublicIp( );

    uint16_t GetPublicPort( );

    void SendPacket( const Packet & ); // Send Packet to WarHawk Server

    std::chrono::steady_clock::time_point GetLastPacketTime( );

  protected:

  private:

    //
    // Methods
    //

    void OnReceivePacket( const Packet &packet_ ); // Receive Packet from WarHawkServer

    //
    // Data
    //

    std::string      m_Name;
    std::string      m_PublicIpAddress;
    const int        m_PublicPort;
    ServerList      &m_ServerList;
    Network         &m_Network;
    PacketProcessor                                  &m_ProxyToClientPacketProcessor;
    std::unique_ptr< warhawk::net::UdpNetworkSocket > m_ProxyToServerSocket;
    std::unique_ptr< PacketProcessor >                m_ProxyToServerPacketProcessor;
    std::chrono::steady_clock::time_point m_LastPacketTime;
};


using ClientPlayerPtr = std::unique_ptr< ClientPlayer >;
