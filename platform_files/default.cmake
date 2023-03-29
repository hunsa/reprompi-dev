
# default configuration 
find_package(MPI REQUIRED)

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)
SET(CMAKE_C_FLAGS "-O2 -Wall -std=c99 -D_POSIX_C_SOURCE=200809L")
SET(CMAKE_CXX_FLAGS "-O2 -Wall -std=c++14")

SET(BUILD_SHARED_LIBS 1)

message(STATUS "Using default compiler: ${CMAKE_C_COMPILER}")