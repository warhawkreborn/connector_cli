#pragma once

//
// This class defines the high-level API to:
// Download WarHawk server list.
// Check forwarding capability of local WarHawk server.
// Publish (AddHost) a local WarHawk server to the list of available public
// servers.
//

#include <vector>

#include "server_entry.h"

class Server;


namespace warhawk
{

class API
{
  public:

    static std::vector< ServerEntry > DownloadServerList( Server * );

    struct ForwardingResponse
    {
      std::string m_ip;
      std::string m_state;
    };

    static ForwardingResponse CheckForwarding( );

    static std::string AddHost( std::string hostname, std::string uniqueId, bool persistent );

  private:

};

} // End namespace warhawk
