#pragma once

//
// A MessageHandler allows registered subscribers to handle processing of particular
// message.
#pragma once

//
// The MessageHandler handles incoming packets.
//

#include "packet.h"
#include "udp_network_socket.h"


class MessageHandler
{
  public:

    virtual ~MessageHandler( )
    {
    }

    virtual void OnReceivePacket( sockaddr_storage client, const Packet & ) = 0;

  protected:

  private:
};
