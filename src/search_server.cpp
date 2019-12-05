#include <thread>

#include "net.h"
#include "search_server.h"

SearchServer::SearchServer( Server *server_ )
  : m_mutex( )
  , m_entries( )
  , m_server( server_ )
{
  m_server->Register( this );
}


SearchServer::~SearchServer( )
{
  m_server->Unregister( this );
}


void SearchServer::run( )
{
  while ( true )
  {
    std::cout << "SearchServer: Searching for new servers to publish." << std::endl;

    {
      std::mutex &mutex = m_server->GetMutex();
      std::unique_lock< std::mutex > lck( mutex );

      // TODO: Do anything that needs to be done here for receiving packets while the mutex is locked.
    }

    std::this_thread::sleep_for( std::chrono::seconds( 30 ) );
  }
}


void SearchServer::OnReceivePacket( struct sockaddr_in client_, std::vector< uint8_t > data_ )
{
}
