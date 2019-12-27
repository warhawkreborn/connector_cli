#pragma once

#include <functional>
#include <list>
#include <string>
#include <thread>

#include "blocking_queue.h"

class Actor
{
  public:

    typedef std::function< void( ) > Message;

    Actor( const std::string &name );
    ~Actor( );

    const std::string &GetName( );
    void SetName( const std::string & );

    void Send( Message );

    static void SetSingleThreaded( bool value = true );

  protected:

  private:

    //
    // Declarations
    //


    //
    // Methods
    //

    Actor( const Actor & );           // No copying.
    void operator =( const Actor & ); // No copying.

    static void Instance( );
    static void Destroy( );

    void Run( );

    void ProcessAllMessageQueues( );
    void ProcessEntireMessageQueue( );
    void ProcessOneMessage( );

    void ProcessMessage( Message & );

    //
    // Data
    //

    // Statics
    static bool                    s_SingleThreaded;

    typedef std::list< Actor * >   ActorList;
    static ActorList              *s_ActorList;

    // Non-statics
    std::string                    m_Name;
    bool                           m_Done;

    typedef enum STATE
    {
      WAITING,
      PROCESSING
    } STATE;

    STATE                          m_State;

    BlockingQueue< Message >       m_MsgQueue;
    std::unique_ptr< std::thread > m_Thread;
};
