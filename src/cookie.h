#pragma once

//
// A Cookie is a piece of information that has a name, path, domain, value and expiration
// time (along with a couple of other pieces of information).
// It is used to store information on remote clients so that when the reconnect, the
// server will be able to identify them and then make use of that information.
//

#include <ostream>
#include <string>
#include <set>
#include <tuple>

namespace warhawk
{

namespace common
{

struct cookie
{
  //
  // Methods
  //

  static cookie parse( const std::string & );

  std::string to_string( ) const;

  //
  // Data
  //

  std::string m_domain;
  std::string m_path;
  std::string m_name;
  std::string m_value;
  bool        m_include_sub;
  bool        m_secure;
  time_t      m_expires;

}; // End struct cookie

bool operator <( const cookie &a, const cookie &b );

struct cookie_list
{
  //
  // Methods
  //

  std::set< cookie > find( const std::string &name, const std::string &domain = "", const std::string &path_ = "" );

  void dump( std::ostream & ) const;

  auto begin(  );
  auto end(    );
  auto begin(  ) const;
  auto end(    ) const;
  auto cbegin( ) const;
  auto cend(   ) const;
  auto rbegin( );
  auto rend(   );
  auto rbegin( ) const;
  auto rend(   ) const;

  //
  // Data
  //

  std::set< cookie > m_cookies;

}; // End struct cookie_list

} // End namespace common

} // End namespace warhawk
