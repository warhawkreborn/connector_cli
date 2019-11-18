cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn )

if ( NOT CMAKE_BUILD_TYPE )

  set( CMAKE_BUILD_TYPE Debug )

endif( ) 

message( "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )

if ( APPLE )

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.10" )
  set( CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -pthread -lc++" )

  # Standard library.
  add_definitions( -stdlib=libc++ )

  # C++14
  add_definitions( -std=c++17 )

  # Warnings as errors.
  # add_definitions( -Werror )

endif( )

add_definitions( -DPICOJSON_USE_INT64 )

set( SOURCE_FILES
     src/main.cpp
     src/net.cpp
     src/webclient.cpp
   )

set( INCLUDE_FILES
     src/cookie.h
     src/net.h
     src/picojson.h
     src/warhawk.h
     src/webclient.h
   )

add_executable( bridge
                ${SOURCE_FILES}
                ${INCLUDE_FILES}
              )


target_link_libraries( bridge
                       curl
                       dl
                       z
                     )
