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

    static std::string CheckForwarding( );

  private:

};

} // End namespace warhawk

#endif // WARHAWK_API_H
