#pragma once

//
// This server periodically queries the server that holds a list of available
// public servers and updates that list into the SearchServer and ForwardServer.
//

class ForwardServer;
class Server;
class SearchServer;

class RequestServer
{
  public:

    RequestServer( ServerList &, PacketProcessor & );
    ~RequestServer( );

    void run( );

  protected:

  private:

    ServerList       &m_ServerList;
    PacketProcessor  &m_PacketProcessor;

    bool           m_Done = false;

    // Make sure this is always last so that the thread destructs (joins) first.
    std::thread    m_Thread;
};
