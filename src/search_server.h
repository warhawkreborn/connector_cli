#pragma once

//
// The SearchServer periodically broadcasts a request for local network servers.
// Once it broadcasts, it listens for one second for any responses.
// Any responses it gets are then sent on to the remote server list server to publish
// as available public servers.
//

#include <iostream>
#include <functional>
#include <list>
#include <vector>

#include "discovery_packet.h"
#include "message_handler.h"
#include "udp_server.h"
#include "server.h"
#include "server_entry.h"
#include "warhawk_api.h"


class SearchServer : public MessageHandler
{
  public:

    //
    // Declarations
    //

    typedef struct PacketData
    {
      PacketData( const std::string &address_, const warhawk::DiscoveryPacket &data_ )
        : m_address( address_ ), m_data( data_ )
      {
      }

      std::string m_address;
      warhawk::DiscoveryPacket m_data;
    } PacketData;

    using PacketList = std::list< PacketData >;

    typedef struct LocalServerData
    {
      LocalServerData( const PacketData &data_, const warhawk::API::ForwardingResponse &response_ )
        : m_PacketData( data_ )
        , m_Response( response_ )
      {
      }

      PacketData                       m_PacketData;
      warhawk::API::ForwardingResponse m_Response;
    } LocalServerData;

    using LocalServerList = std::vector< LocalServerData >;

    //
    // Methods
    //

    SearchServer( Server * );
    ~SearchServer( );

    void run( );

    void OnReceivePacket( sockaddr_storage client, std::vector< uint8_t > data ) override;

    void SetEntries( std::vector< ServerEntry > e_ );

    bool LocalServerContainsIp( const std::string &ip ); // True if the list contains this IP.

    void ForEachServer( std::function< void ( const LocalServerData & ) > );

  protected:

  private:

    //
    // Methods
    //

    void DoStateWaiting( );
    void DoStateBroadcasting( );
    void DoStateCollecting( );
    void DoStateProcessing( );

    void ForEachServerNoLock( std::function< void( const LocalServerData & ) > );

    //
    // Data
    //

    const std::string m_DiscoveryPacket =
      "c381b800001900b6018094004654000005000000010000000000020307000000c0a814ac000000002d27000000000000010000005761726861"
      "776b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002801800ffffffff000000"
      "00000000004503d7e0000000000000005a";

    std::mutex                  m_mutex;
    std::vector< ServerEntry >  m_entries;
    Server                     *m_server = nullptr;

    enum class STATE
    {
      STATE_WAITING,
      STATE_BROADCASTING,
      STATE_COLLECTING,
      STATE_PROCESSING,
    };

    STATE m_CurrentState;


    PacketList  m_PacketList;

    LocalServerList m_LocalServers;

    bool        m_Done = false;

    // Make sure this is always last so that the thread destructs (joins) first.
    std::thread m_Thread;
};
