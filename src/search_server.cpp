#include <thread>

#include "net.h"
#include "search_server.h"

SearchServer::SearchServer( warhawk::net::udp_server &udpServer_ )
  : Server( udpServer_ )
{
}


void SearchServer::run( )
{
  while ( true )
  {
    std::cout << "SearchServer: Searching for new servers to publish." << std::endl;

    {
      std::mutex &mutex = GetMutex();
      std::unique_lock< std::mutex > lck( mutex );

      // TODO: Do anything that needs to be done here for receiving packets while the mutex is locked.
    }

    std::this_thread::sleep_for( std::chrono::seconds( 30 ) );
  }
}
