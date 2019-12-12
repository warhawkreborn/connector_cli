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

add_executable( warhawk-reborn
                ${SOURCE_FILES}
                ${INCLUDE_FILES}
              )


target_link_libraries( warhawk-reborn
                       ${PLATFORM_LIBS}
                       ${CMAKE_THREAD_LIBS_INIT}
                     )

install(TARGETS warhawk-reborn
        RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dominik Thalhammer <dominik@thalhammer.it>")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Connector to allow discovery of warhawk servers")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/warhawkreborn/connector_unix")

include(CPack)