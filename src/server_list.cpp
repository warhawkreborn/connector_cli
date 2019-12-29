#include "server_list.h"


ServerList::ServerList( )
{
}


ServerList::~ServerList( )
{
}


void ServerList::AddRemoteServerEntries( const std::vector< ServerEntry > &list_ )
{
  std::unique_lock< std::mutex > guard( m_mutex );

  // First, delete any remote entries.
  ForEachServerNoLock( [ ] ( ServerEntry &entry_ )
  {
    if ( !entry_.m_LocalServer )
    {
      entry_.m_DeleteEntry = true;
    }

    const bool continueOn = true;
    return continueOn;
  } );

  // Now add remote entries from list, but only if they don't match public IP with local server.
  for ( ServerEntry entry : list_ )
  {
    if ( !ContainsLocalServerWithIpNoLock( entry.m_ip ) )
    {
      m_ServerList.push_back( entry );
    }
  }
}


void ServerList::AddLocalServerEntries( const std::vector< ServerEntry > &list_ )
{
  std::unique_lock< std::mutex > guard( m_mutex );

  // First, delete any local entries.
  ForEachServerNoLock( [ ] ( ServerEntry &entry_ )
  {
    if ( entry_.m_LocalServer )
    {
      entry_.m_DeleteEntry = true;
    }

    const bool continueOn = true;
    return continueOn;
  } );

  for ( auto &entry : list_ )
  {
    ServerEntry newEntry = entry;
    newEntry.m_ip   = entry.m_PublicIpResponse.m_ip;
    newEntry.m_name = entry.m_PacketData.m_data.GetName( );

    // Now add list of servers.
    m_ServerList.push_back( newEntry );
  }
}
 

void ServerList::ForEachServer( std::function< bool ( ServerEntry & ) > func_ )
{
  std::unique_lock< std::mutex > guard( m_mutex );

  ForEachServerNoLock( func_ );
}


void ServerList::ForEachServerNoLock( std::function< bool( ServerEntry & ) > func_ )
{
  for ( LocalServerList::iterator itr = m_ServerList.begin( ); itr != m_ServerList.end( ); )
  {
    ServerEntry &entry = *itr;
    bool continueOn = func_( entry );

    if ( entry.m_DeleteEntry )
    {
      itr = m_ServerList.erase( itr );
    }
    else
    {
      ++itr;
    }

    if ( !continueOn )
    {
      break;
    }
  }
}


bool ServerList::ContainsLocalServerWithIp( const std::string &ip_ )
{
  std::unique_lock< std::mutex > guard( m_mutex );

  return ContainsLocalServerWithIpNoLock( ip_ );
}


bool ServerList::ContainsLocalServerWithIpNoLock( const std::string &ip_ )
{
  bool found = false;

  ForEachServerNoLock( [ & ] ( ServerEntry &entry_ )
  {
    bool continueOn = true;

    if ( ip_ == entry_.m_PublicIpResponse.m_ip )
    {
      found = true;
      continueOn = false;
    }

    return continueOn;
  } );

  return found;
}
