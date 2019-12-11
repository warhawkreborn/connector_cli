cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn )

add_definitions( -DPICOJSON_USE_INT64 )

if ( USE_SYSTEM_CURL )

  find_package( CURL REQUIRED )

endif( )

find_package( Threads )

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
     version.h
   )

set( WARHAWK_TARGET warhawk-reborn )

add_executable( ${WARHAWK_TARGET}
                ${SOURCE_FILES}
                ${INCLUDE_FILES}
              )


target_include_directories( ${WARHAWK_TARGET} PRIVATE ${CMAKE_BINARY_DIR} )
target_include_directories( ${WARHAWK_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_include_directories( ${WARHAWK_TARGET} PRIVATE ${CURL_INCLUDE_DIR} )

target_link_libraries( ${WARHAWK_TARGET}
                       ${PLATFORM_LIBS}
                       ${CMAKE_THREAD_LIBS_INIT}
                     )

# Create version.

add_custom_target( gitversion ALL
                   DEPENDS ${CMAKE_BINARY_DIR}/version.h
                 )

add_custom_command( OUTPUT ${CMAKE_BINARY_DIR}/version.h
                    COMMAND ${CMAKE_COMMAND}
                    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
                    -DBINARY_DIR=${CMAKE_BINARY_DIR}
                    -P ${CMAKE_SOURCE_DIR}/scripts/git.cmake
                  )

# version.h is a generated file.
set_source_files_properties( ${CMAKE_BINARY_DIR}/version.h
                             PROPERTIES GENERATED TRUE
                             HEADER_FILE_ONLY TRUE
                     )


add_dependencies( ${WARHAWK_TARGET} gitversion )
