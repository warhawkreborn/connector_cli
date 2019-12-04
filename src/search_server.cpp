#include <thread>

#include "net.h"
#include "search_server.h"


SearchServer::SearchServer( warhawk::net::udp_server &udpServer_ )
  : m_server( udpServer_ )
{

}

void SearchServer::run( )
{
  while ( true )
  {
    std::cout << "SearchServer: Searching for new servers to publish." << std::endl;

    std::this_thread::sleep_for( std::chrono::seconds( 30 ) );
  }
}
