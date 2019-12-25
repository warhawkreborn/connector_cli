// Copied and improved from uWebSockets/examples/helpers.

// This is just a very simple and inefficient demo of async responses,
// please do roll your own variant or use a database or Node.js's async
// features instead of this really bad demo.

#include <cstring>
#include <future>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>


struct AsyncFileReader
{
  public:

    /* Construct a demo async. file reader for fileName */
    AsyncFileReader( std::string fileName_ )
      : m_FileName( fileName_ )
    {
      m_Fin.open( m_FileName, std::ios::binary );

      // get fileSize
      m_Fin.seekg( 0, m_Fin.end );
      m_FileSize = m_Fin.tellg( );

      // std::cout << "File size is: " << fileSize << std::endl;

      // cache up 1 mb!
      m_Cache.resize( 1024 * 1024 );

      // std::cout << "Caching 1 MB at offset = " << 0 << std::endl;
      m_Fin.seekg( 0, m_Fin.beg );
      m_Fin.read( m_Cache.data( ), m_Cache.length( ) );
      m_CacheOffset = 0;
      m_HasCache = true;

      // get loop for thread

      m_Loop = uWS::Loop::get( );
    }

    /* Returns any data already cached for this offset */
    std::string_view peek( int offset_ )
    {
      /* Did we hit the cache? */
      if ( m_HasCache && offset_ >= m_CacheOffset && ( ( offset_ - m_CacheOffset ) < m_Cache.length() ) )
      {
        /* Cache hit */
        // std::cout << "Cache hit!" << std::endl;

        /*if (fileSize - offset < cache.length()) {
            std::cout << "LESS THAN WHAT WE HAVE!" << std::endl;
        }*/

        int chunkSize = std::min< int >( m_FileSize - offset_, m_Cache.length( ) - offset_ + m_CacheOffset );

        return std::string_view( m_Cache.data( ) + offset_ - m_CacheOffset, chunkSize );
      }
      else
      {
        /* Cache miss */
        // std::cout << "Cache miss!" << std::endl;
        return std::string_view( nullptr, 0 );
      }
    }

    /* Asynchronously request more data at offset */
    void request( int offset_, std::function< void( std::string_view ) > cb_ )
    {
      // in this case, what do we do?
      // we need to queue up this chunk request and callback!
      // if queue is full, either block or close the connection via abort!
      if ( !m_HasCache )
      {
        // already requesting a chunk!
        std::cout << "ERROR: already requesting a chunk!" << std::endl;
        return;
      }

      // disable cache
      m_HasCache = false;

      std::async( std::launch::async, [ this, cb_, offset_ ] ( )
      {
        // std::cout << "ASYNC Caching 1 MB at offset = " << offset << std::endl;

        // den har stängts! öppna igen!
        if ( !m_Fin.good() )
        {
          m_Fin.close( );
          // std::cout << "Reopening fin!" << std::endl;
          m_Fin.open( m_FileName, std::ios::binary );
        }

        m_Fin.seekg( offset_, m_Fin.beg );
        m_Fin.read( m_Cache.data(), m_Cache.length() );

        m_CacheOffset = offset_;

        m_Loop->defer( [ this, cb_, offset_ ] ( )
        {
          int chunkSize = std::min< int >( m_Cache.length( ), m_FileSize - offset_ );

          // båda dessa sker?
          if ( chunkSize == 0 )
          {
            std::cout << "Zero size!?" << std::endl;
          }

          if ( chunkSize != m_Cache.length( ) )
          {
            std::cout << "LESS THAN A CACHE 1 MB!" << std::endl;
          }

          m_HasCache = true;
          cb_( std::string_view( m_Cache.data( ), chunkSize ) );
        } );
      } );
    }

    /* Abort any pending async. request */
    void abort( )
    {
    }

    int getFileSize( )
    {
      return m_FileSize;
    }

  private:

    /* The cache we have in memory for this file */
    std::string m_Cache       = "";
    int         m_CacheOffset = 0;
    bool        m_HasCache    = false;

    /* The pending async file read (yes we only support one pending read) */
    std::function< void( std::string_view ) > m_PendingReadCb = nullptr;

    int           m_FileSize = 0;
    std::string   m_FileName = "";
    std::ifstream m_Fin      { };
    uWS::Loop    *m_Loop     = nullptr;
};
