#ifndef MESSAGE_SERVER_H
#define MESSAGE_SERVER_H

#include <vector>

#include "net.h"

class MessageHandler
{
  public:

    virtual ~MessageHandler( )
    {
    }

    virtual void OnReceivePacket( struct sockaddr_in client, std::vector< uint8_t > data ) = 0;

  protected:

  private:
};


#endif // MESSAGE_SERVER_H
