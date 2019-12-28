#include "server_list.h"


ServerList::ServerList( )
{
}


ServerList::~ServerList( )
{
}


void ServerList::AddRemoteServerEntries( std::vector< ServerEntry > &list_ )
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


void ServerList::AddLocalServerEntry( const std::string &ip_, const ServerEntry &entry_ )
{
  std::unique_lock< std::mutex > guard( m_mutex );

  bool found = false;

  if ( ContainsLocalServerWithIpNoLock( entry_.m_PublicIpResponse.m_ip ) )
  {
    found = true;
  }

  if ( !found )
  {
    m_ServerList.push_back( entry_ );
  }
}


void ServerList::UpdateLocalServerEntry( const std::string &ip_, const ServerEntry &entry_ )
{
  std::unique_lock< std::mutex > guard( m_mutex );

  ForEachServerNoLock( [ & ] ( ServerEntry &searchEntry_ )
  {
    bool continueOn = true;

    if ( ip_ == searchEntry_.m_PublicIpResponse.m_ip )
    {
      searchEntry_.m_LocalServer      = entry_.m_LocalServer;
      searchEntry_.m_PacketData       = entry_.m_PacketData;
      searchEntry_.m_PublicIpResponse = entry_.m_PublicIpResponse;
      continueOn = false;
    }

    return continueOn;
  } );
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
