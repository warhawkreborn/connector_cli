#include "packet.h"


Packet::Packet( )
{
}


Packet::Packet( std::vector< uint8_t > data_ )
  : m_Data( data_ )
{
  SetType( );
}


Packet::~Packet( )
{
}


Packet::TYPE Packet::GetType( ) const
{
  return m_Type;
}


std::vector< uint8_t > &Packet::GetData( )
{
  return m_Data;
}


const std::vector< uint8_t > &Packet::GetData( ) const
{
  return m_Data;
}


void Packet::SetType( )
{
  int debug = 0;

  if ( m_Data.size( ) == 0 )
  {
    return;
  }
  else if ( m_Data.size( ) == 188 && GetData( )[ 0 ] == 0xc3 && GetData( )[ 1 ] == 0x81 )
  {
    m_Type = TYPE::TYPE_SERVER_INFO_REQUEST;
  }
  else if ( m_Data.size( ) == 372 && GetData( )[ 0 ] == 0xc3 && GetData( )[ 1 ] == 0x81 )
  {
    m_Type = TYPE::TYPE_SERVER_INFO_RESPONSE;
  }
  else if ( m_Data.size( ) == 19 && GetData( )[ 0 ] == 0xc4 && GetData( )[ 1 ] == 0x81 )
  {
    m_Type = TYPE::TYPE_GAME_CLIENT_TO_SERVER;
  }
  else if ( m_Data.size( ) < 40 )
  {
    debug = 2;
  }
  else
  {
    debug = 1;
  }
}
