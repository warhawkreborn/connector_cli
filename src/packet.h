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
      TYPE_UNKNOWN,
      TYPE_SERVER_INFO_REQUEST,
      TYPE_SERVER_INFO_RESPONSE,
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
