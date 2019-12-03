#pragma once

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

  static cookie parse( const std::string &n_ )
  {
    cookie res;
    auto peq  = n_.find( '='  );
    auto ptab = n_.find( '\t' );

    if ( ptab < peq )
    {
      // Netscape
      res.m_domain = n_.substr( 0, ptab );
      auto start = ptab + 1;
      ptab = n_.find( '\t', start );
      res.m_include_sub = n_.substr( start, ptab - start ) != "FALSE";
      start = ptab + 1;
      ptab = n_.find( '\t', start );
      res.m_path = n_.substr( start, ptab - start );
      start = ptab + 1;
      ptab = n_.find( '\t', start );
      res.m_secure = n_.substr( ptab, ptab - start ) != "FALSE";
      start = ptab + 1;
      ptab = n_.find( '\t', start );
      res.m_expires = std::stoull( n_.substr( start, ptab - start ) );
      start = ptab + 1;
      ptab = n_.find( '\t', start );
      res.m_name = n_.substr( start, ptab - start );
      start = ptab + 1;
      res.m_value = n_.substr( start );
    }
    else
    {
      if ( peq == std::string::npos )
      {
        throw std::invalid_argument( "Unknown format" );
      }

      res.m_name  = n_.substr( 0, peq );
      res.m_value = n_.substr( peq + 1 );
    }

    return res;
  }

  std::string to_string( ) const
  {
    std::string res = m_domain + "\t";
    res += m_include_sub ? "TRUE\t" : "FALSE\t";
    res += m_path + "\t";
    res += m_secure ? "TRUE\t" : "FALSE\t";
    res += std::to_string( m_expires ) + "\t";
    res += m_name + "\t";
    res += m_value;
    return res;
  }

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

inline bool operator <( const cookie &a_, const cookie &b_ )
{
  return std::tie( a_.m_domain, a_.m_path, a_.m_name, a_.m_include_sub ) <
         std::tie( b_.m_domain, b_.m_path, b_.m_name, b_.m_include_sub );
}

struct cookie_list
{
  //
  // Methods
  //

  std::set< cookie > find( const std::string &name_, const std::string &domain_ = "", const std::string &path_ = "" )
  {
    std::set< cookie > res;

    for ( auto &c : m_cookies )
    {
      if ( c.m_name != name_ )
      {
        continue;
      }

      if( !domain_.empty( ) && c.m_domain != domain_ )
      {
        continue;
      }

      if( !path_.empty( ) && c.m_path != path_ )
      {
        continue;
      }

      res.insert( c );
    }

    return res;
  }

  void dump( std::ostream &str_ ) const 
  {
    str_ << "Total cookies:" << m_cookies.size( ) << "\n";

    for ( auto& e : m_cookies )
    {
      str_ << e.to_string( ) << "\n";
    }
  }

  auto begin(  )       { return m_cookies.begin(  ); }
  auto end(    )       { return m_cookies.end(    ); }
  auto begin(  ) const { return m_cookies.begin(  ); }
  auto end(    ) const { return m_cookies.end(    ); }
  auto cbegin( ) const { return m_cookies.cbegin( ); }
  auto cend(   ) const { return m_cookies.cend(   ); }
  auto rbegin( )       { return m_cookies.rbegin( ); }
  auto rend(   )       { return m_cookies.rend(   ); }
  auto rbegin( ) const { return m_cookies.rbegin( ); }
  auto rend(   ) const { return m_cookies.rend(   ); }

  //
  // Data
  //

  std::set< cookie > m_cookies;

}; // End struct cookie_list

} // End namespace common

} // End namespace warhawk
