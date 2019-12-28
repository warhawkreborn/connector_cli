#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>

//
// Data structure that holds the associated information for each server.
//

struct ServerEntry
{
  std::string            m_name;
  int                    m_ping = 0;
  std::vector< uint8_t > m_frame;
  std::string            m_ip;
  bool                   m_LocalServer = false;
};


class ServerList
{
  public:

    ServerList( );
    ~ServerList( );

    void AddEntry( const ServerEntry & );

    void ForEachServer( std::function< bool ( const ServerEntry & ) > );

  protected:

  private:

  std::mutex      m_mutex; // Protectes the LocalServerList.
  using LocalServerList = std::vector< ServerEntry >;
  LocalServerList m_ServerList;
};
