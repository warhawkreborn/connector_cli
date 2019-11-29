cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn )

option( USE_SYSTEM_CURL "Use the system CURL (not the built-in one)." OFF )

if ( NOT CMAKE_BUILD_TYPE )

  set( CMAKE_BUILD_TYPE Debug )

endif( ) 

message( "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )

if ( APPLE )

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.10" )
  set( CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -lc++" )

  # Standard library.
  add_definitions( -stdlib=libc++ )

  # C++14
  add_definitions( -std=c++14 )

  # Warnings as errors.
  # add_definitions( -Werror )

  set( USE_SYSTEM_CURL ON )

elseif ( MSVC )

  set( USE_SYSTEM_CURL OFF )

else( )

  set( USE_SYSTEM_CURL ON )

endif( )


if ( NOT USE_SYSTEM_CURL )

  add_subdirectory( libs/curl )

endif( )

add_subdirectory( src )
