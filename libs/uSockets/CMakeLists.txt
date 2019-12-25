cmake_minimum_required( VERSION 3.6.3 )

project( uSockets )

set( SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/uSockets-0.3.4/src )

add_definitions( -DLIBUS_NO_SSL )

set( SOURCE_FILES
     ${SRC_DIR}/context.c
     ${SRC_DIR}/loop.c
     ${SRC_DIR}/socket.c
     ${SRC_DIR}/ssl.c
   )

set( INCLUDE_FILES
     ${SRC_DIR}/libusockets.h
   )

set( USOCKETS_TARGET ${PROJECT_NAME} )

add_library( ${USOCKETS_TARGET}
             ${SOURCE_FILES}
           )

target_include_directories( ${USOCKETS_TARGET} PRIVATE ${SRC_DIR} )

target_link_libraries( ${USOCKETS_TARGET}
                       ${PLATFORM_LIBS}
                       uv_a
                       ${CMAKE_THREAD_LIBS_INIT}
                     )
