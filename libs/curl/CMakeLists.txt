# CURL configuration

project( CURL )

set( BUILD_CURL_EXE    OFF CACHE BOOL "Don't build CURL.EXE"         )
set( BUILD_SHARED_LIBS OFF CACHE BOOL "Don't build shared libs"      )
set( BUILD_TESTING     OFF CACHE BOOL "Don't build testing projects" )
set( CURL_STATIC_CRT   ON  CACHE BOOL "DO build static library"      )
set( ENABLE_MANUAL     OFF CACHE BOOL "Don't build the manual"       )
set( HTTP_ONLY         ON  CACHE BOOL "Do build HTTP_ONLY code"      )

if ( WIN32 )

  set( CMAKE_USE_WINSSL ON CACHE BOOL "Do build with WIN SLL"        )

endif( )

add_subdirectory( curl-7.67.0 )
