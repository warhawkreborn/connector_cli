cmake_minimum_required( VERSION 3.6.3 )

project( WarHawkReborn VERSION 1.1.0 )

find_package( Git )
if ( NOT Git_FOUND )

  message( FATAL "GIT not found." )

endif( )

execute_process( COMMAND
                 "${GIT_EXECUTABLE}" rev-parse --short HEAD
                 WORKING_DIRECTORY "${SOURCE_DIR}"
                 OUTPUT_VARIABLE GIT_HASH
                 ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
               )

# The date of the commit.
execute_process( COMMAND
                 "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
                 WORKING_DIRECTORY "${SOURCE_DIR}"
                 OUTPUT_VARIABLE GIT_DATE
                 ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
               )

add_definitions( -DPROJECT_VERSION=${PROJECT_VERSION} )
add_definitions( -DGIT_HASH=${GIT_HASH}               )
add_definitions( -DGIT_DATE=${GIT_DATE}               )

if ( WARHAWK_BUILD )

  add_definitions( -DWARHAWK_BUILD=${WARHAWK_BUILD} )
  message( "WARHAWK_BUILD = ${WARHAWK_BUILD}" )

endif( )

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

set( WARHAWK_TARGET warhawkreborn )

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

install(TARGETS ${WARHAWK_TARGET}
        RUNTIME DESTINATION bin)

install(FILES warhawkreborn.service
        DESTINATION "/lib/systemd/system/"
        COMPONENT "warhawkreborn")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dominik Thalhammer <dominik@thalhammer.it>")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Connector to allow discovery of warhawk servers")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/warhawkreborn/connector_unix")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst")

include(CPack)
