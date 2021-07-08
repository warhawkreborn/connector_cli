#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "discovery_packet.h"
#include "warhawk_api.h"

//
// Data structure that holds the associated information for each server.
//

typedef struct PacketData
{
  PacketData( )
    : m_address( )
    , m_data( )
  {
  }

  PacketData( const std::string &address_, const warhawk::DiscoveryPacket &data_ )
    : m_address( address_ )
    , m_data( data_ )
  {
  }

  std::string              m_address;
  warhawk::DiscoveryPacket m_data;
} PacketData;


typedef struct ServerEntry
{
  std::string                      m_name;
  int                              m_ping = 0;
  std::vector< uint8_t >           m_frame;
  std::string                      m_ip;
  bool                             m_LocalServer = false;
  PacketData                       m_PacketData;
  warhawk::API::ForwardingResponse m_PublicIpResponse;
  bool                             m_DeleteEntry = false;
} ServerEntry;


class ServerList
{
  public:

    ServerList( );
    ~ServerList( );

    void AddRemoteServerEntries( const std::vector< ServerEntry > & );
    void AddLocalServerEntries(  const std::vector< ServerEntry > & );

    void ForEachServer( std::function< bool ( ServerEntry & ) > );

    bool ContainsLocalServerWithIp( const std::string &ip );

  protected:

  private:

    //
    // Methods
    //

    void ForEachServerNoLock( std::function< bool( ServerEntry & ) > );

    bool ContainsLocalServerWithIpNoLock( const std::string &ip );

    //
    // Data
    //

    std::mutex      m_mutex; // Protectes the LocalServerList.
    using LocalServerList = std::vector< ServerEntry >;
    LocalServerList m_ServerList;
};
