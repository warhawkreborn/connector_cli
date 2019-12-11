cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn )

find_package( Git )

# The commit's hash.
execute_process( COMMAND
                 "${GIT_EXECUTABLE}" rev-parse --short HEAD
                 WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                 OUTPUT_VARIABLE GIT_HASH
                 ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
               )

# The date of the commit.
execute_process( COMMAND
                 "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
                 WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                 OUTPUT_VARIABLE GIT_DATE
                 ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
               )

# generate version.cpp
message( "GIT_HASH=${GIT_HASH}" )
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in"
                "${CMAKE_CURRENT_BINARY_DIR}/version.cpp"
                @ONLY
              )


add_definitions( -DPICOJSON_USE_INT64 )

if ( USE_SYSTEM_CURL )

  find_package( CURL REQUIRED )

endif( )

find_package( Threads )

include_directories( ${CURL_INCLUDE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

if ( APPLE OR UNIX )

  set( PLATFORM_LIBS 
       curl
       dl
       z
     )

else( )

  add_definitions( -D_WINSOCK_DEPRECATED_NO_WARNINGS )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS         )

  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT"  )
  set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   /MTd" )

  set( PLATFORM_LIBS
       libcurl
       ws2_32
       iphlpapi
     )

endif( )

set( SOURCE_FILES
     addr_info.cpp
     discovery_packet.cpp
     forward_server.cpp
     main.cpp
     net.cpp
     network.cpp
     search_server.cpp
     server.cpp
     warhawk_api.cpp
     webclient.cpp
   )

set( INCLUDE_FILES
     addr_info.h
     cookie.h
     discovery_packet.h
     forward_server.h
     message_handler.h
     net.h
     network.h
     picojson.h
     search_server.h
     server.h
     server_entry.h
     warhawk.h
     warhawk_api.h
     webclient.h
   )

list( APPEND SOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" version.h )

add_executable( warhawk-reborn
                ${SOURCE_FILES}
                ${INCLUDE_FILES}
              )


target_link_libraries( warhawk-reborn
                       ${PLATFORM_LIBS}
                       ${CMAKE_THREAD_LIBS_INIT}
                     )
