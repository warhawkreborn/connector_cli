#include <iostream>

#include "actor.h"


bool Actor::s_SingleThreaded = false;
Actor::ActorList *Actor::s_ActorList = NULL;


Actor::Actor( const std::string &name_ )
  // m_SingleThreaded
  // m_ActorList
  : m_Name( name_ )
  , m_Done( false )
  , m_State( WAITING )
  , m_MsgQueue( )
  , m_Thread( )
{
  Instance( );

  s_ActorList->push_back( this );

  if ( !s_SingleThreaded )
  {
    m_Thread = std::unique_ptr< std::thread >( new std::thread( [=]
    {
      this->Run( );
    } ) );
  }
}


Actor::~Actor( )
{
  Instance( );

  Send( [&]
  {
    m_Done = true;
  } );

  if ( !s_SingleThreaded )
  {
    m_Thread->join( );
  }
}


void Actor::SetSingleThreaded( bool value_ )
{
  Instance( );

  s_SingleThreaded = value_;
}


void Actor::Run( )
{
  ProcessEntireMessageQueue( );
}


void Actor::ProcessEntireMessageQueue( )
{
  Instance( );

  while ( !m_Done )
  {
    // Return if single-threaded and MsgQueue is empty.
    if ( s_SingleThreaded && m_MsgQueue.size( ) == 0 )
    {
      return;
    }


    ProcessOneMessage( );
  }
}


void Actor::ProcessOneMessage( )
{
  Message msg;

  // Otherwise, block on waiting for message.
  if ( m_MsgQueue.Take( msg ) )
  {
    m_State = PROCESSING;
    // Now process the message.
    ProcessMessage( msg );
    m_State = WAITING;
  }
}


void Actor::ProcessAllMessageQueues( )
{
  Instance( );

  if ( s_SingleThreaded )
  {
    for ( ActorList::iterator itr = s_ActorList->begin( );
          itr != s_ActorList->end( );
          ++itr )
    {
      Actor *actor = *itr;
      actor->ProcessEntireMessageQueue( );
    }
  }
}


void Actor::ProcessMessage( Message &msg_ )
{
  msg_( );
}


void Actor::Send( Message m_ )
{
  Instance( );

  m_MsgQueue.Add( m_ );

  if ( s_SingleThreaded )
  {
    ProcessAllMessageQueues( );
  }
}


const std::string &Actor::GetName( )
{
  return m_Name;
}


void Actor::SetName( const std::string &name_ )
{
  m_Name = name_;
}


void Actor::Instance( )
{
  if ( s_ActorList == NULL )
  {
    s_ActorList = new ActorList;
    s_SingleThreaded = false;
  }
}


void Actor::Destroy( )
{
  if ( s_ActorList != NULL )
  {
    delete s_ActorList;
    s_ActorList = NULL;
  }
}
