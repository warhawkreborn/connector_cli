cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn )

add_definitions( -DPICOJSON_USE_INT64 )

if ( USE_SYSTEM_CURL )

  find_package( CURL REQUIRED )

endif( )

find_package( Threads )

include_directories( ${CURL_INCLUDE_DIR} )

if ( APPLE OR UNIX )

  set( UNIX_LIBS 
       curl
       dl
       z
     )

else( )

  set( WIN_LIBS
       libcurl
     )

endif( )

set( SOURCE_FILES
     main.cpp
     net.cpp
     webclient.cpp
   )

set( INCLUDE_FILES
     cookie.h
     net.h
     picojson.h
     warhawk.h
     webclient.h
   )

add_executable( warhawk-reborn
                ${SOURCE_FILES}
                ${INCLUDE_FILES}
              )


target_link_libraries( warhawk-reborn
                       ${UNIX_LIBS}
                       ${WIN_LIBS}
                       ${CMAKE_THREAD_LIBS_INIT}
                     )
