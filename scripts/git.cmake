message( "*** Executing GIT commands to generate version.h file." )

find_package( Git )

# The commit's hash.
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

message( "GIT: SOURCE_DIR=${SOURCE_DIR}" )
message( "GIT: BINARY_DIR=${BINARY_DIR}" )

# generate version.h file.
message( "GIT_HASH=${GIT_HASH}" )
configure_file( "${SOURCE_DIR}/src/version.h.in"
                "${BINARY_DIR}/version.h.txt"
                @ONLY
              )

# Copy the file to the final header only if the version changes, which
# reduces needless rebuilds.
execute_process( COMMAND ${CMAKE_COMMAND} -E copy_if_different
                 ${BINARY_DIR}/version.h.txt
                 ${BINARY_DIR}/version.h
               )
