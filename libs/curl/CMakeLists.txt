# CURL configuration

project( CURL )

set( BUILD_CURL_EXE    OFF )
set( BUILD_SHARED_LIBS OFF )
set( CURL_STATIC_CRT   ON  )
set( HTTP_ONLY         ON  )

if ( WIN32 )

  set( CMAKE_USE_WINSSL ON )

endif( )

add_subdirectory( curl-7.67.0 )
