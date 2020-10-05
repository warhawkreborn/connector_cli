#include "packet.h"


Packet::Packet( )
{
}


Packet::Packet( std::vector< uint8_t > data_, bool fromLocalNetwork_ )
  : m_Data( data_ )
  , m_FromLocalNetwork( fromLocalNetwork_ )
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
  size_t size = m_Data.size( );
  bool validPacket = size >= 4 && ValidPacket( );

  if ( size == 0 )
  {
    return;
  }
  else if ( size == 188 && validPacket && GetData( )[ 0 ] == 0xc3 && GetData( )[ 1 ] == 0x81 )
  {
    m_Type = TYPE::TYPE_SERVER_INFO_REQUEST;
  }
  else if ( size == 372 && validPacket && GetData( )[ 0 ] == 0xc3 && GetData( )[ 1 ] == 0x81 )
  {
    m_Type = TYPE::TYPE_SERVER_INFO_RESPONSE;
  }
  else if ( !m_FromLocalNetwork )
  {
    m_Type = TYPE::TYPE_GAME_CLIENT_TO_SERVER;
  }
  else if ( m_FromLocalNetwork )
  {
    m_Type = TYPE::TYPE_GAME_SERVER_TO_CLIENT;
  }
  else
  {
    int debug = 1;
  }
}


const struct sockaddr_storage &Packet::GetClient( ) const
{
  return m_Client;
}


void Packet::SetClient( const struct sockaddr_storage &client_ )
{
  m_Client = client_;
}


std::string Packet::GetIp( ) const
{
  return m_Ip;
}


void Packet::SetIp( const std::string &ip_ )
{
  m_Ip = ip_;
}


uint16_t Packet::GetPort( ) const
{
  return m_Port;
}


void Packet::SetPort( const uint16_t &port_ )
{
  m_Port = port_;
}


bool Packet::GetFromLocalNetwork( ) const
{
  return m_FromLocalNetwork;
}


void Packet::SetFromLocalNetwork( const bool &value_ )
{
  m_FromLocalNetwork = value_;
}


bool Packet::ValidPacket( )
{
  if ( GetData( ).size( ) == 0 )
  {
    return false;
  }

  if ( GetData( ).size( ) < 4 )
  {
    return false;
  }

  uint16_t len = GetData( )[ 3 ];
  len = ( len << 8 ) | GetData( )[ 2 ];

  if ( GetData( ).size( ) - 4 != len )
  {
    return false;
  }

  return true;
}
