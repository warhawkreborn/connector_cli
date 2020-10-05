// Copied and improved from uWebSockets/examples/helpers.

/* Middleware to fill out content-type */
inline bool hasExt( std::string_view file_, std::string_view ext_)
{
  if ( ext_.size( ) > file_.size( ) )
  {
    return false;
  }

  return std::equal( ext_.rbegin( ), ext_.rend( ), file_.rbegin( ) );
}

/* This should be a filter / middleware like app.use(handler) */
template < bool SSL >
uWS::HttpResponse< SSL > *serveFile( uWS::HttpResponse< SSL > *res_, uWS::HttpRequest *req_ )
{
  res_->writeStatus( uWS::HTTP_200_OK );

  if ( hasExt( req_->getUrl( ), ".svg" ) )
  {
    res_->writeHeader( "Content-Type", "image/svg+xml" );
  }

  return res_;
}
