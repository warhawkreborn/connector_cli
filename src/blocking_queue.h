#pragma once

#include <cstdint>
#include <condition_variable>
#include <deque>
#include <limits>
#include <mutex>


// Blocking Queue based on the Java's implementation.
// This class is thread safe.
template< typename WorkType >
class BlockingQueue
{
  public:

    enum
    {
      NoMaxSizeRestriction = 0
    };

    // Default constructor.
    // Provides no size restriction on the blocking queue.
    BlockingQueue( )
      : m_MaxSize( NoMaxSizeRestriction )
      , m_Queue( )
      , m_Mutex( )
      , m_DataAvailable( )
      , m_NotFull( )
      , m_Stop( false )
    {
    }

    // Retrieve and remove the oldest element in the queue. 
    // If there is no element available, this method will block until
    // elements are inserted.
    //
    // This method is thread safe.
    // Returns true if not stopped, else false.
    bool Take( WorkType &result_ )
    {
      std::unique_lock< std::mutex > guard( m_Mutex );

      if ( m_Queue.empty( ) == true )
      {
        m_DataAvailable.wait( guard, [&] ( )
        {
          // Waits until this returns true.
          return m_Stop || !m_Queue.empty( );
        } );
      }

      if ( m_Stop )
      {
        return false; // Stopped.
      }

      result_ = m_Queue.back( );

      m_Queue.pop_back( );

      m_NotFull.notify_one( ); // If the queue was full, it should not be now.

      return true; // Not stopped.
    }

    // Insert a new element to the blocking queue. 
    // Return true if the operation is successful, false otherwise.
    //
    // This method is thread safe.
    bool Add( WorkType const & workUnit_ )
    {
      std::unique_lock< std::mutex > guard( m_Mutex );

      if ( ( m_Queue.size( ) >= m_MaxSize ) && 
           ( NoMaxSizeRestriction != m_MaxSize ) )
      {
        m_NotFull.wait( guard, [&] ()
        {
          // Wait until this returns true.
          return m_Stop || m_Queue.size( ) < m_MaxSize;
        } );
      }

      if ( m_Stop )
      {
        return false;
      }

      m_Queue.push_front( workUnit_ );

      m_DataAvailable.notify_one( );

      return true;
    }

    size_t size( ) const
    {
      return m_Queue.size( );
    }

    // return the maximum allowed size for this queue.
    uint64_t GetMaxSize( ) const
    {
      return m_MaxSize;
    }

    // set the maximum allowed size for this queue.
    void SetMaxSize( uint64_t val )
    {
      m_MaxSize = val;
    }

    void Stop( )
    {
      m_Stop = true;
      m_DataAvailable.notify_one( );
    }

  protected:

  private:

    //
    // Declarations
    //

    //
    // Data
    //

    uint64_t                m_MaxSize;

    std::deque< WorkType >  m_Queue;
    std::mutex              m_Mutex;
    std::condition_variable m_DataAvailable; // Fired when data is put into the queue.
    std::condition_variable m_NotFull;       // Fired when the queue is not full.
    bool                    m_Stop;
};
