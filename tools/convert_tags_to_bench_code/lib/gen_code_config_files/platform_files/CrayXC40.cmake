
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

SET(CMAKE_C_COMPILER cc)
SET(CMAKE_CXX_COMPILER CC)
SET(CMAKE_C_FLAGS "-O3")

SET(BUILD_SHARED_LIBS 0)

SET(LIBRARY_SUFFIX "a")

message(STATUS "Using Cray compilers: ${CMAKE_C_COMPILER}")
