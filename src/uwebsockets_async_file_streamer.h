// Copied and improved from uWebSockets/examples/helpers.

#include <filesystem>
#include <sstream>


struct AsyncFileStreamer
{
  //
  // Methods
  //

  AsyncFileStreamer( std::string root_ )
    : m_Root( root_ )
  {
    // for all files in this path, init the map of AsyncFileReaders
    updateRootCache( );
  }

  void updateRootCache( )
  {
      // todo: if the root folder changes, we want to reload the cache
      for( auto &p : std::filesystem::recursive_directory_iterator( m_Root ) )
      {
        std::string url = p.path( ).string( ).substr( m_Root.length( ) );
        if ( url == "/index.html" )
        {
            url = "/";
        }

        char *key = new char[ url.length( ) ];
        memcpy( key, url.data( ), url.length( ) );

        m_AsyncFileReaders[ std::string_view( key, url.length( ) ) ] = new AsyncFileReader( p.path( ).string( ) );
      }
  }

  template < bool SSL >
  void streamFile( uWS::HttpResponse< SSL > *res_, std::string_view url_ )
  {
    auto it = m_AsyncFileReaders.find( url_ );

    if ( it == m_AsyncFileReaders.end( ) )
    {
      std::stringstream ss;
      ss << "Did not find file: " << url_;
      throw std::runtime_error( ss.str( ) );
    }
    else
    {
      streamFile( res_, it->second );
    }
  }

  template < bool SSL >
  static void streamFile( uWS::HttpResponse< SSL > *res_, AsyncFileReader *asyncFileReader_ )
  {
    /* Peek from cache */
    std::string_view chunk = asyncFileReader_->peek( res_->getWriteOffset( ) );
    if ( !chunk.length( ) || res_->tryEnd( chunk, asyncFileReader_->getFileSize( ) ).first )
    {
      /* Request new chunk */
      // todo: we need to abort this callback if peer closed!
      // this also means Loop::defer needs to support aborting (functions should embedd an atomic boolean abort or something)

      // Loop::defer(f) -> integer
      // Loop::abort(integer)

      // hmm? no?

      // us_socket_up_ref eftersom vi delar ägandeskapet

      if ( chunk.length( ) < asyncFileReader_->getFileSize( ) )
      {
        asyncFileReader_->request( res_->getWriteOffset( ), [ res_, asyncFileReader_ ] ( std::string_view chunk_ )
        {
          // check if we were closed in the mean time
          //if (us_socket_is_closed()) {
              // free it here
              //return;
          //}

          /* We were aborted for some reason */
          if ( !chunk_.length( ) )
          {
            // todo: make sure to check for is_closed internally after all callbacks!
            res_->close( );
          }
          else
          {
            AsyncFileStreamer::streamFile( res_, asyncFileReader_ );
          }
        });
      }
    }
    else
    {
      /* We failed writing everything, so let's continue when we can */
      res_->onWritable( [ res_, asyncFileReader_ ]( int offset_ )
      {

        // här kan skiten avbrytas!

        AsyncFileStreamer::streamFile( res_, asyncFileReader_ );
        // todo: I don't really know what this is supposed to mean?
        return false;
      })->onAborted( [ ] ( )
      {
        std::cout << "ABORTED!" << std::endl;
      });
    }
  }

  //
  // Data
  //

  std::map< std::string_view, AsyncFileReader * > m_AsyncFileReaders;
  std::string                                     m_Root;
};
