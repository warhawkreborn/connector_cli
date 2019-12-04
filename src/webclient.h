#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#pragma once

#include <map>

#include "cookie.h"

namespace warhawk
{

namespace common
{

struct response
{
  std::string                               m_data;
  cookie_list                               m_cookies;
  std::multimap< std::string, std::string > m_headers;
  long                                      m_status_code;
};

struct request
{
	std::string                               m_url;
	std::string                               m_method;
	std::string                               m_data;
	cookie_list                               m_cookies;
	std::multimap< std::string, std::string > m_headers;
	bool                                      m_follow_redirect;

	static request default_get(  std::string url );
	static request default_post( std::string url, std::string data );
};


class webclient
{
  public:

    webclient( );
    ~webclient( );

    webclient( const webclient & ) = delete;
    webclient &operator =( const webclient & ) = delete;

    void set_verbose( bool );

    response execute( const request & );

  private:

    void *m_curl;
};

} // End common namespace

} // End warhawk namespace

#endif // WEBCLIENT_H
