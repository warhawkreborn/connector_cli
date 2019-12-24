#pragma once

//
// A MessageHandler allows registered subscribers to handle processing of particular
// message.
//

#include <vector>

#include "udp_server.h"

class MessageHandler
{
  public:

    virtual ~MessageHandler( )
    {
    }

    virtual void OnReceivePacket( sockaddr_storage client, std::vector< uint8_t > data ) = 0;

  protected:

  private:
};
