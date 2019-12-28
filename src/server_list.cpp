#include "server_list.h"


ServerList::ServerList( )
{
}


ServerList::~ServerList( )
{
}


void ServerList::AddEntry( const ServerEntry &entry_ )
{
  std::unique_lock< std::mutex > guard( m_mutex );
}


void ServerList::ForEachServer( std::function< bool ( const ServerEntry & ) > func_ )
{
  std::unique_lock< std::mutex > guard( m_mutex );

  for ( const auto &entry : m_ServerList )
  {
    bool continueOn = func_( entry );
    if ( !continueOn )
    {
      break;
    }
  }
}
