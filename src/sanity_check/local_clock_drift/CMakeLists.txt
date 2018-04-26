
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
   
project(clock-drift)

# default configuration 
find_package(MPI REQUIRED)

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)
SET(CMAKE_C_FLAGS "-O3 -Wall ") # -pedantic")

set(SRC_DIR "${CMAKE_SOURCE_DIR}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")


find_package(GSL REQUIRED)
if (GSL_INCLUDE_DIR)
message (STATUS "GSL INCLUDES: ${GSL_INCLUDE_DIR}")
else(GSL_INCLUDE_DIR)
message (FATAL_ERROR "GSL libraries not found.")
endif(GSL_INCLUDE_DIR)


INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR} "src")
set(COMMON_LIBRARIES ${GSL_LIBRARIES} )


SET(MY_COMPILE_FLAGS "")


add_executable(clock_drift
${SRC_DIR}/measure_local_clock_drift.c
${SRC_DIR}/parse_test_options.c
)
TARGET_LINK_LIBRARIES(clock_drift ${COMMON_LIBRARIES} )
SET_TARGET_PROPERTIES(clock_drift PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")

