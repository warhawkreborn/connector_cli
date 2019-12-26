cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn VERSION 1.1.0 )

find_package( Git )
if ( NOT Git_FOUND )

  message( FATAL "GIT not found." )

endif( )

if ( "${GIT_HASH}" STREQUAL "" )

  message( "Finding GIT parameters" )

  execute_process( COMMAND
                   "${GIT_EXECUTABLE}" rev-parse --short HEAD
                   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                   OUTPUT_VARIABLE GIT_HASH
                   ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
                 )

endif( )

message( "GIT_HASH = ${GIT_HASH}" )

add_definitions( -DPROJECT_VERSION=${PROJECT_VERSION} )
add_definitions( -DGIT_HASH=${GIT_HASH}               )

if ( WARHAWK_BUILD )

  add_definitions( -DWARHAWK_BUILD=${WARHAWK_BUILD} )
  message( "WARHAWK_BUILD = ${WARHAWK_BUILD}" )

endif( )

if ( USE_SYSTEM_CURL )

  find_package( CURL REQUIRED )

endif( )

find_package( Threads )

if ( APPLE )

  set( PLATFORM_LIBS 
       curl
       dl
       z
     )

elseif ( UNIX )

  set( PLATFORM_LIBS 
       curl
       dl
       z
       stdc++fs
     )


else( )

  add_definitions( -D_WINSOCK_DEPRECATED_NO_WARNINGS )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS         )

  set( CMAKE_CPP_FLAGS_RELEASE "${CMAKE_CPP_FLAGS_RELEASE} /MT"  )
  set( CMAKE_CPP_FLAGS_DEBUG   "${CMAKE_CPP_FLAGS_DEBUG}   /MTd" )

  set( PLATFORM_LIBS
       libcurl
       ws2_32
       iphlpapi
     )

endif( )

set( SOURCE_FILES
     addr_info.cpp
     cookie.cpp
     discovery_packet.cpp
     forward_server.cpp
     http_server.cpp
     main.cpp
     network.cpp
     request_server.cpp
     search_server.cpp
     server.cpp
     udp_server.cpp
     warhawk_api.cpp
     webclient.cpp
   )

set( INCLUDE_FILES
     addr_info.h
     cookie.h
     discovery_packet.h
     forward_server.h
     http_server.h
     message_handler.h
     network.h
     picojson.h
     request_server.h
     search_server.h
     server.h
     server_entry.h
     udp_server.h
     uwebsockets_async_file_reader.h
     uwebsockets_async_file_streamer.h
     uwebsockets_middleware.h
     warhawk.h
     warhawk_api.h
     webclient.h
   )

set( WARHAWK_TARGET warhawkreborn )

add_executable( ${WARHAWK_TARGET}
                ${SOURCE_FILES}
                ${INCLUDE_FILES}
              )


target_include_directories( ${WARHAWK_TARGET} PRIVATE ${CMAKE_BINARY_DIR} )
target_include_directories( ${WARHAWK_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_include_directories( ${WARHAWK_TARGET} PRIVATE ${CURL_INCLUDE_DIR} )
target_include_directories( ${WARHAWK_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/libs/uWebSockets/uWebSockets-0.16.5/src )
target_include_directories( ${WARHAWK_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/libs/uSockets/uSockets-0.3.4/src )

target_link_libraries( ${WARHAWK_TARGET}
                       ${PLATFORM_LIBS}
                       uSockets
                       uv_a
                       ${CMAKE_THREAD_LIBS_INIT}
                     )

if ( UNIX )

  install( TARGETS ${WARHAWK_TARGET}
           RUNTIME DESTINATION bin
         )

else( )

  install( TARGETS ${WARHAWK_TARGET}
           RUNTIME DESTINATION .
         )

endif( )

if ( UNIX )

  install( FILES warhawkreborn.service
           DESTINATION "/lib/systemd/system/"
           COMPONENT   "warhawkreborn"
         )

endif( )

if ( UNIX )

  set( CPACK_GENERATOR "DEB" )

endif( )

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dominik Thalhammer <dominik@thalhammer.it>")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Connector to allow discovery of warhawk servers")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/warhawkreborn/connector_unix")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${GIT_HASH}" )

include(CPack)
