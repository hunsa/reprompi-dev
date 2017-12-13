# - Find the hwloc library
# This module sets the following variables:
#  HWLOC_FOUND - set to true if a library is found
#  HWLOC_LINKER_FLAGS - uncached list of required linker flags (excluding -l
#    and -L).
#  HWLOC_LIBRARIES - uncached list of libraries (using full path name) to
#    link against to use HWLOC
#  HWLOC_INCLUDE_DIR - directory where the HWLOC header files are
#
##########

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)

set(HWLOC_LIBRARY_NAME "hwloc")

find_path (HWLOC_INCLUDE_DIR ${HWLOC_LIBRARY_NAME}.h
              PATHS ${HWLOC_LIBRARY_DEFAULT_PATH}
              #PATHS ENV LD_LIBRARY_PATH DYLD_LIBRARY_PATH
              PATH_SUFFIXES include
              NO_DEFAULT_PATH
              NO_CMAKE_ENVIRONMENT_PATH
          	  NO_CMAKE_PATH
              NO_SYSTEM_ENVIRONMENT_PATH
         	  NO_CMAKE_SYSTEM_PATH
              )

find_library (HWLOC_LIBRARY ${HWLOC_LIBRARY_NAME}
              PATHS ${HWLOC_LIBRARY_DEFAULT_PATH}
              #PATHS ENV LD_LIBRARY_PATH DYLD_LIBRARY_PATH
              PATH_SUFFIXES lib
              NO_DEFAULT_PATH
              NO_CMAKE_ENVIRONMENT_PATH
          	  NO_CMAKE_PATH
              NO_SYSTEM_ENVIRONMENT_PATH
         	  NO_CMAKE_SYSTEM_PATH
              )

if (HWLOC_LIBRARY)
    message(STATUS "HWLOC library path: ${HWLOC_LIBRARY}" )       
else(HWLOC_LIBRARY)
    message(STATUS "HWLOC library path: not found" )
endif()



include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set HWLOC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(HWLOC DEFAULT_MSG
                                  HWLOC_LIBRARY)

mark_as_advanced(HWLOC_INCLUDE_DIR HWLOC_LIBRARY )
set(HWLOC_INCLUDE_DIRS ${HWLOC_INCLUDE_DIR} )
set(HWLOC_LIBRARIES ${HWLOC_LIBRARY} )


