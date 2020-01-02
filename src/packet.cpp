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
  if ( m_Data.size( ) == 188 )
  {
    m_Type = TYPE::TYPE_SERVER_INFO_REQUEST;
  }
  else if ( m_Data.size( ) == 372 )
  {
    m_Type = TYPE::TYPE_SERVER_INFO_RESPONSE;
  }
}
