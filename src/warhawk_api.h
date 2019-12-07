#ifndef WARHAWK_API_H
#define WARHAWK_API_H

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

#endif // WARHAWK_API_H
