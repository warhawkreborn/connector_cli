cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn )

add_definitions( -DPICOJSON_USE_INT64 )

if ( USE_SYSTEM_CURL )

  find_package( CURL REQUIRED )

endif( )

find_package( Threads )

include_directories( ${CURL_INCLUDE_DIR} )

if ( APPLE OR UNIX )

  set( PLATFORM_LIBS 
       curl
       dl
       z
     )

else( )

  add_definitions( -D_WINSOCK_DEPRECATED_NO_WARNINGS )

  set( PLATFORM_LIBS
       libcurl
     )

endif( )

set( SOURCE_FILES
     forward_server.cpp
     main.cpp
     net.cpp
     webclient.cpp
   )

set( INCLUDE_FILES
     cookie.h
     forward_server.h
     net.h
     picojson.h
     server_entry.h
     warhawk.h
     webclient.h
   )

add_executable( warhawk-reborn
                ${SOURCE_FILES}
                ${INCLUDE_FILES}
              )


target_link_libraries( warhawk-reborn
                       ${PLATFORM_LIBS}
                       ${CMAKE_THREAD_LIBS_INIT}
                     )
