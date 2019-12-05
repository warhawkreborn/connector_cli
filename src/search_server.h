#ifndef SEARCH_SERVER_H
#define SEARCH_SERVER_H

#include <iostream>

#include "net.h"
#include "server.h"
#include "server_entry.h"

class SearchServer : public Server
{
  public:

    SearchServer( warhawk::net::udp_server &udpServer_ );

    void run( ) override;

  protected:

  private:

    const std::string m_DiscoveryPacket =
      "c381b800001900b6018094004654000005000000010000000000020307000000c0a814ac000000002d27000000000000010000005761726861"
      "776b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002801800ffffffff000000"
      "00000000004503d7e0000000000000005a";

    std::mutex m_mtx;
    std::vector< ServerEntry > m_entries;
};

#endif // SEARCH_SERVER_H
