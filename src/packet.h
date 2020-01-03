#pragma once

//
// The WarHawk packets are classified according to different types, this is the base type.
//

#include <memory>
#include <vector>


class Packet
{
  public:

    enum class TYPE
    {
      // Powers of two so they can also be 'or'ed together to form groups.
      TYPE_UNKNOWN               = 0x0,
      TYPE_SERVER_INFO_REQUEST   = 0x1,
      TYPE_SERVER_INFO_RESPONSE  = 0x2,
      TYPE_GAME_CLIENT_TO_SERVER = 0x4,
      TYPE_GAME_SERVER_TO_CLIENT = 0x8,
    };

    Packet( );
    Packet( std::vector< uint8_t > data );
    virtual ~Packet( );

    TYPE GetType( ) const;

    std::vector< uint8_t > &GetData( );
    const std::vector< uint8_t > &GetData( ) const;

  protected:

  private:

    //
    // Methods
    //

     void SetType( );

    //
    // Data
    //

    TYPE                   m_Type = TYPE::TYPE_UNKNOWN;
    std::vector< uint8_t > m_Data;
};


using PacketPtr = std::shared_ptr< Packet >;
