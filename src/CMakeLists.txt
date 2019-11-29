cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn )

add_definitions( -DPICOJSON_USE_INT64 )

find_package( CURL REQUIRED )

find_package( Threads )

include_directories( ${CURL_INCLUDE_DIR} )

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

add_executable( bridge
                ${SOURCE_FILES}
                ${INCLUDE_FILES}
              )


target_link_libraries( bridge
                       curl
                       dl
                       z
                       ${CMAKE_THREAD_LIBS_INIT}
                     )
