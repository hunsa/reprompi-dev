
cmake_minimum_required(VERSION 3.22)

include(FetchContent)
include(ExternalProject)
include(GNUInstallDirs)

SET(PGTUNE_CMAKE_ARGS "")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH 1)

find_package(GSL REQUIRED)
if (GSL_INCLUDE_DIRS)
    message (STATUS "GSL INCLUDES : ${GSL_INCLUDE_DIRS}")
    message (STATUS "GSL LIBRARIES: ${GSL_LIBRARIES}")
else(GSL_INCLUDE_DIRS)
    message (FATAL_ERROR "GSL libraries not found.")
endif(GSL_INCLUDE_DIRS)


#INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR} "src")
#set(COMMON_LIBRARIES ${GSL_LIBRARIES} )

if(OPTION_ENABLE_LOGGING)
    SET(PGTUNE_CMAKE_ARGS "-DOPTION_ENABLE_LOGGING=ON ${PGTUNE_OPTIONS}")
    message(STATUS "Logging for pgtunelib enabled, cmake args: ${PGTUNE_CMAKE_ARGS}")
else()
    message(STATUS "Logging for pgtunelib disabled")
endif()

#set(OFFLINE_BUILD_MODE off CACHE BOOL "download a fresh copy of all dependencies")

#if(OPTION_ENABLE_CIRCULANTS)
#    set(GIT_SUBMODULES_CIRCULANTS GIT_SUBMODULES submodules/mpi_circulants)
#    set(PGTUNE_CMAKE_ARGS "-DOPTION_ENABLE_CIRCULANTS=ON ${PGTUNE_CMAKE_ARGS}")
#    message(STATUS "Enabling circulant collectives in pgtunelib")
#else()
#    set(GIT_SUBMODULES_CIRCULANTS "")
#    message(STATUS "Circulant collectives in pgtunelib disabled")
#endif()

#if (OFFLINE_BUILD_MODE)
#    message(STATUS "disable git pull")
#    ExternalProject_Add(pgtunelib
#            GIT_REPOSITORY https://github.com/hunsa/pgmpitunelib
#            GIT_TAG master
#            #GIT_SUBMODULES submodules/mpi-lane-collectives
#            #${GIT_SUBMODULES_CIRCULANTS}
#            CMAKE_ARGS -DINCLUDE_PLATFORM_CONFIG_FILE=${ABS_INCLUDE_PLATFORM_CONFIG_FILE} ${PGTUNE_CMAKE_ARGS} -DOPTION_ENABLE_ALGID_STORING=OFF
#            PREFIX ${CMAKE_SOURCE_DIR}/external/
#            DOWNLOAD_COMMAND ""
#            UPDATE_DISCONNECTED 1
#    )
#else()
#    message(STATUS "enable git pull")

#FetchContent_Declare(
#        MPI_LANE_COLLECTIVES
#        GIT_REPOSITORY https://github.com/parlab-tuwien/mpi-lane-collectives.git
#        GIT_TAG main
#)
#
#FetchContent_MakeAvailable(MPI_LANE_COLLECTIVES)
#
#FetchContent_Declare(
#        MPI_CIRCULANTS
#        GIT_REPOSITORY git@github.com:parlab-tuwien/mpi-circulant-collectives.git
#        GIT_TAG main
#)
#
#FetchContent_MakeAvailable(MPI_CIRCULANTS)
#
##    ExternalProject_Add(pgtunelib
##            GIT_REPOSITORY https://github.com/hunsa/pgmpitunelib
##            GIT_TAG master
##            #GIT_SUBMODULES submodules/mpi-lane-collectives
##            #${GIT_SUBMODULES_CIRCULANTS}
##            CMAKE_ARGS -DINCLUDE_PLATFORM_CONFIG_FILE=${ABS_INCLUDE_PLATFORM_CONFIG_FILE} ${PGTUNE_CMAKE_ARGS} -DOPTION_ENABLE_ALGID_STORING=OFF
##            PREFIX ${CMAKE_SOURCE_DIR}/external/
##    )
#
#export(EXPORT mpicirc-targets FILE "${PROJECT_BINARY_DIR}/mpicirc-targets.cmake")
#set(MPICIRC_CONFIG ${CMAKE_INSTALL_PREFIX}/share/cmake/mpicirc/mpicirc-config.cmake)
#set(PATH_CIRCULANTS ${CIRC_INSTALL_LIBDIR})  # Library target for MPI_CIRCULANTS
#message(STATUS "PATH_CIRCULANTS: ${PATH_CIRCULANTS}")
##add_definitions(-DPATH_CIRCULANTS=${PATH_CIRCULANTS})

#FetchContent_Declare(
#        PGTUNELIB
#        GIT_REPOSITORY https://github.com/hunsa/pgmpitunelib
#        #GIT_TAG master
#        GIT_TAG fetchcontent
#)
#
#
#FetchContent_MakeAvailable(PGTUNELIB)
#message(STATUS "PGTUNELIB available: ${PGTUNELIB}")
#
#
#set(PGTUNE_CONFIG_CSV ${install_dir}/src/pgtunelib-build/pgmpi_conf.csv)
#add_custom_target(config
#        COMMAND  bash -c "exec ./bin/pgmpi_info > ${PGTUNE_CONFIG_CSV}"
#        VERBATIM
#)

set(PGTUNELIB_PATH "" CACHE STRING "Path to pgmpicli library")
find_path(PGTUNELIB_INCLUDE_DIR pgmpi_tune.h PATHS ${PGTUNELIB_PATH}/include)
find_library(PGTUNELIB_LIB NAMES pgmpicli PATHS ${PGTUNELIB_PATH}/lib)

#message(STATUS "foo: ${PGTUNELIB_PATH}/share/cmake/pgmpi/PGMPITargets.cmake")
#export(EXPORT pgmpicli-targets FILE "${PGTUNELIB_PATH}/share/cmake/pgmpi/PGMPITargets.cmake")
#include("${PGTUNELIB_PATH}/share/cmake/pgmpi/PGMPITargets.cmake")

#set(PGMPICLI_DIR "${PGTUNELIB_PATH}/share/cmake/pgmpi/")
#list(APPEND CMAKE_MODULE_PATH "${PGTUNELIB_PATH}/share/cmake/")
#list(APPEND CMAKE_PREFIX_PATH "${PGTUNELIB_PATH}/share/cmake/")
#find_package(PGMPICLI CONFIG REQUIRED PATHS "${PGTUNELIB_PATH}/share/cmake/pgmpi/")
#find_package(PGMPICLI REQUIRED PATHS "${PGTUNELIB_PATH}/share/cmake/pgmpi/")

#find_library(PGMPICLI NAMES pgmpicli)

#if (NOT TARGET PGMPICLI::pgmpicli)
#    message(FATAL_ERROR "FAILED TO FIND PGMPI")
#endif()


if(NOT PGTUNELIB_INCLUDE_DIR)
	message(FATAL_ERROR "pgmpi_tune.h not found. Please set PGTUNELIB_PATH to enable this library.")
else(NOT PGTUNELIB_INCLUDE_DIR)
    message(STATUS "pgmpi_tune.h found in ${PGTUNELIB_PATH}/include.")
endif()

if(NOT PGTUNELIB_LIB)
    message(FATAL_ERROR "pgmpicl not found. Please set PGTUNELIB_PATH to enable this library.")
else(NOT PGTUNELIB_LIB)
    message(STATUS "pgmpicli found: ${PGTUNELIB_LIBRARIES}.")
endif()

set(PGTUNE_CONFIG_CSV ${CMAKE_INSTALL_PREFIX}/pgmpi_conf.csv)
add_custom_target(config
        COMMAND  bash -c "exec ${PGTUNELIB_PATH}/bin/pgmpi_info > ${PGTUNE_CONFIG_CSV}"
        VERBATIM
)


list(APPEND CMAKE_BUILD_RPATH "${PGTUNELIB_PATH}/lib")

add_executable(pgchecker
        pgchecker.cpp
        utils/csv_parser.cpp
        pgcheck_helper.cpp
        pgcheck_options.cpp
        pgtunelib_interface.cpp
        pgdata.cpp
        pgdata_table.cpp
        pgdata_comparer.cpp
        pgdata_printer.cpp
        comparer/statistical_test/ttest.cpp
        comparer/statistical_test/wilcoxon_rank_sum.cpp
        comparer/statistical_test/wilcoxon_mann_whitney.cpp
        comparer/statistical_test/two_sample_test_factory.cpp
        comparer/statistical_test/two_sample_test.cpp
        comparer/comparer_data.cpp
        comparer/comparer_factory.cpp
        comparer/simple_comparer.cpp
        comparer/violation/detailed_violation_comparer.cpp
        comparer/violation/violation_comparer.cpp
        comparer/violation/grouped_violation_comparer.cpp
        comparer/runtime/abs_runtime_comparer.cpp
        comparer/runtime/rel_runtime_comparer.cpp
        comparer/raw/raw_comparer.cpp
        logger/logger.cpp
        pgcheck_input.cpp
        utils/string_utils.cpp
        utils/time_utils.cpp
        utils/argv_manager.cpp

        ${SRC_DIR}/benchmark_job.c
        ${SRC_DIR}/benchmarkCollective.c
        ${SRC_DIR}/reprompi_bench/misc.c
        ${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
        # synchronization methods
        ${SYNC_SRC_FILES}
        # output
        ${COMMON_OUTPUT_MAN_SRC_FILES}
        ${SRC_DIR}/reprompi_bench/output_management/results_output.c
        # command-line options
        ${COMMON_OPTION_PARSER_SRC_FILES}
        ${SRC_DIR}/reprompi_bench/option_parser/parse_timing_options.c
        #${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
        ${SRC_DIR}/reprompi_bench/option_parser/parse_options.c
        # collective functions
        ${COLL_OPS_SRC_FILES}
        # caches cleaning strategy
        ${CACHING_SRC_FILES}
)


target_include_directories(pgchecker PRIVATE
        ${GSL_INCLUDE_DIRS}
        ${PGTUNELIB_INCLUDE_DIR})
target_link_directories(pgchecker PRIVATE "${PGTUNELIB_PATH}/lib")

#SET_TARGET_PROPERTIES(pgchecker PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS} -I${PGTUNE_INC_DIR}")
#SET_TARGET_PROPERTIES(pgchecker PROPERTIES LINK_FLAGS " -L${PGTUNELIB_PATH}/lib")
TARGET_LINK_LIBRARIES(pgchecker ${GSL_LIBRARIES} pgmpicli MPITS::mpits MPI::MPI_CXX)
#target_include_directories(pgchecker PRIVATE ${pgmpitunelib_SOURCE_DIR}/include)
#SET_TARGET_PROPERTIES(pgchecker PROPERTIES LINKER_LANGUAGE CXX )


