cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn )

option( USE_SYSTEM_CURL "Use the system CURL (not the built-in one)." OFF )

if ( NOT CMAKE_BUILD_TYPE )

  set( CMAKE_BUILD_TYPE Debug )

endif( ) 

message( "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )

if ( APPLE )

  # C++17
  add_definitions( -std=c++17 )

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.10" )
  set( CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -lc++" )

  # Standard library.
  add_definitions( -stdlib=libc++ )

  # Warnings as errors.
  # add_definitions( -Werror )

  set( USE_SYSTEM_CURL ON )

elseif ( MSVC )

  add_definitions( /std:c++17  /Zc:__cplusplus )

  set( USE_SYSTEM_CURL OFF )

else( )

  # C++17
  add_definitions( -std=c++17 )

  set( USE_SYSTEM_CURL ON )

endif( )


if ( NOT USE_SYSTEM_CURL )

  # Build the library but don't install any of it's targets.
  add_subdirectory( libs/curl EXCLUDE_FROM_ALL )

endif( )

add_subdirectory( src )
