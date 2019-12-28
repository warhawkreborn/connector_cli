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
#include "udp_server.h"

class SearchServer;


class ForwardServer : public MessageHandler
{
  public:

    ForwardServer( PacketServer *, SearchServer & );
    ~ForwardServer( );
 
    void SetEntries( std::vector< ServerEntry > );

    void OnReceivePacket( sockaddr_storage client, std::vector< uint8_t > data ) override;

    // Iterate through each server entry in the list of servers.
    void ForEachServer( std::function< void ( const ServerEntry & ) > );

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
    PacketServer               *m_PacketServer = nullptr;
    SearchServer               &m_SearchServer;
};
