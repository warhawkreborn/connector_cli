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

  private:

};

} // End namespace warhawk

#endif // WARHAWK_API_H
