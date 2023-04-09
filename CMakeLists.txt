
cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
   
project(reprompi)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")

set(INCLUDE_PLATFORM_CONFIG_FILE "${CMAKE_SOURCE_DIR}/platform_files/default.cmake" 
    CACHE STRING "Configure project to use a specific platform file")
include(${INCLUDE_PLATFORM_CONFIG_FILE})

get_filename_component(ABS_INCLUDE_PLATFORM_CONFIG_FILE ${INCLUDE_PLATFORM_CONFIG_FILE} ABSOLUTE)

find_package(GSL REQUIRED)
if (GSL_INCLUDE_DIR)
message (STATUS "GSL INCLUDES: ${GSL_INCLUDE_DIR}")
else(GSL_INCLUDE_DIR)
message (FATAL_ERROR "GSL libraries not found.")
endif(GSL_INCLUDE_DIR)


INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR} "src")
set(COMMON_LIBRARIES ${GSL_LIBRARIES} )

# option(ENABLE_LOGP_SYNC "Enable O(log(p)) clock synchronization for the HCA algorithm [default: disabled]" off)
option(RDTSC_CALIBRATION "Calibrate the frequency of RTDSC [default: off (uses FREQUENCY_MHZ)]" off)

option(ENABLE_RDTSC "Use RDTSC for time measurements [default: MPI_Wtime()]" off)
option(ENABLE_RDTSCP "Use RDTSCP for time measurements [default: MPI_Wtime()]" off)
option(ENABLE_GETTIME_REALTIME "Use clock_gettime(CLOCK_REALTIME) for time measurements [default: MPI_Wtime()]" off)
option(ENABLE_GETTIME_MONOTONIC "Use clock_gettime(CLOCK_MONOTONIC) for time measurements [default: MPI_Wtime()]" off)
option(ENABLE_DOUBLE_BARRIER "Call barrier twice for synchronization [default: disabled]" off)
set(FREQUENCY_MHZ 2300 CACHE STRING "CPU Frequency (needed for RDTSCP-based time measurements)")

option(COMPILE_BENCH_LIBRARY "Compile ReproMPI library [default: disabled]" off)
option(COMPILE_BENCH_TESTS "Enable benchmark testing [default: disabled]" off)

option(COMPILE_SANITY_CHECK_TESTS "Enable sanity check tests" off)
option(COMPILE_SANITY_CHECK_CLOCK "Enable clock sanity checks" off)
set(OPTION_BUFFER_ALIGNMENT "" CACHE STRING "Alignment to use for buffer allocation [default: no alignment(allocation with calloc)]")
option(OPTION_PRINT_MSIZES_BYTES "Print message sizes in the output instead of MPI call counts [default: counts]" off)

option(OPTION_ENABLE_LOGGING   "Enable logging" off)
option(OPTION_ENABLE_DEBUGGING "Enable debugging" off)
option(OPTION_ENABLE_PGCHECKER "Enable performance guidelines checking" off)

####################################
############ PGtune support ########
####################################
set(PGMPI_LIBRARY_DEFAULT_PATH "" CACHE STRING  "Specify the path of the PGMPI library")
option(ENABLE_PGMPI "Use PGMPI" off)

if (ENABLE_PGMPI)
    find_package(PGMPI)
    
    if(NOT PGMPI_FOUND)
        message(STATUS "PGMPI not found. Skipping the generation of PGMPI-linked benchmark binaries.")
    endif(NOT PGMPI_FOUND)
else(ENABLE_PGMPI)
    message(STATUS "PGMPI disabled.")
endif(ENABLE_PGMPI)
####################################


SET(MY_COMPILE_FLAGS "")

######################################
############ Set timing flags ########
######################################
if(RDTSC_CALIBRATION)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DRDTSC_CALIBRATION")
endif()

if(ENABLE_GETTIME_REALTIME)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_GETTIME_REALTIME")
endif()

if(ENABLE_GETTIME_MONOTONIC)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_GETTIME_MONOTONIC")
endif()

if(FREQUENCY_MHZ)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DFREQUENCY_MHZ=${FREQUENCY_MHZ}")
endif()


#################################
############ Other flags ########
#################################
if(OPTION_BUFFER_ALIGNMENT)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DOPTION_BUFFER_ALIGNMENT=${OPTION_BUFFER_ALIGNMENT}")
endif()

if(OPTION_PRINT_MSIZES_BYTES)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DOPTION_PRINT_MSIZES_BYTES=${OPTION_PRINT_MSIZES_BYTES}")
endif()

if(OPTION_ENABLE_LOGGING)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DUSE_LOGGING")
endif()

if(OPTION_ENABLE_DEBUGGING)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -g")
endif()

####################################
############ HWLOC support ########
####################################
set(HWLOC_LIBRARY_DEFAULT_PATH "" CACHE STRING  "Specify the path of the HWLOC library")

find_package(Hwloc REQUIRED)
if (HWLOC_INCLUDE_DIR)
message (STATUS "HWLOC INCLUDES: ${HWLOC_INCLUDE_DIR}")
SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DHAVE_HWLOC")
else(HWLOC_INCLUDE_DIR)
message (WARNING "The HWLOC library not found. Compiling without hwloc support.")
endif(HWLOC_INCLUDE_DIR)
####################################



set(BUF_MANAGER_SRC_FILES
${SRC_DIR}/buf_manager/mem_allocation.c
)

set(COLL_OPS_SRC_FILES
${SRC_DIR}/collective_ops/collectives.c
${SRC_DIR}/collective_ops/mpi_collectives.c
${SRC_DIR}/collective_ops/pingpong.c
# memory allocation
${BUF_MANAGER_SRC_FILES}
)


set(COMMON_OPTION_PARSER_SRC_FILES
${SRC_DIR}/reprompi_bench/option_parser/parse_common_options.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
)


set(SYNC_SRC_FILES
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/synchronization.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/sync_info.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/hca_sync.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/hca3_sync.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/sk_sync.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/jk_sync.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/no_sync.c
${SRC_DIR}/reprompi_bench/sync/process_sync/barrier_sync.c
${SRC_DIR}/reprompi_bench/sync/process_sync/barrier_sync_common.c
${SRC_DIR}/reprompi_bench/sync/process_sync/dissem_barrier_impl.c
${SRC_DIR}/reprompi_bench/sync/process_sync/dissem_barrier_sync.c
${SRC_DIR}/reprompi_bench/sync/process_sync/double_barrier_sync.c
${SRC_DIR}/reprompi_bench/sync/process_sync/proc_sync_none.c
${SRC_DIR}/reprompi_bench/sync/process_sync/round_time_sync.c
${SRC_DIR}/reprompi_bench/sync/process_sync/round_sync_common.c
${SRC_DIR}/reprompi_bench/sync/process_sync/reprompi_collectives.c
${SRC_DIR}/reprompi_bench/sync/process_sync/process_synchronization.c
${SRC_DIR}/reprompi_bench/sync/process_sync/window_sync.c
#${SRC_DIR}/reprompi_bench/sync/common/sync_module_helpers.c
)


set(CACHING_SRC_FILES
${SRC_DIR}/reprompi_bench/caching/caching.c
${SRC_DIR}/reprompi_bench/caching/warm_cache.c
${SRC_DIR}/reprompi_bench/caching/clear_cache_memset.c
)


# generate commit version
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD RESULT_VARIABLE res_var OUTPUT_VARIABLE GIT_COM_ID )
    if( NOT ${res_var} EQUAL 0 )
        set(GIT_COM_ID "unknown")
        message(STATUS "Current directory is not a git repository. Build will not contain git revision info." )
    endif()
    string( REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COM_ID} )
else()
    set(GIT_COMMIT_ID "unknown_git_not_found")
    message(STATUS "Git command not found. Build will not contain git revision info." )
endif()
configure_file("${SRC_DIR}/version.h.in" "${SRC_DIR}/version.h" @ONLY)


set(COMMON_OUTPUT_MAN_SRC_FILES
${SRC_DIR}/version.h
${SRC_DIR}/reprompi_bench/output_management/bench_info_output.c
${SRC_DIR}/reprompi_bench/output_management/runtimes_computation.c
)

set(CLOCKSYNC_SRC_FILES
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClock.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/MPIClock.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GettimeClock.cpp

${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffset.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.cpp

${SRC_DIR}/reprompi_bench/sync/clock_sync/hca_sync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sk_sync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/jk_sync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_sync_common.cpp
${SRC_DIR}/reprompi_bench/sync/common/sync_module_helpers.c
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_sync_loader.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/synchronization.c
${SRC_DIR}/reprompi_bench/sync/clock_sync/no_sync.c
${SRC_DIR}/reprompi_bench/sync/clock_sync/topo_aware_2l_sync.cpp

${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/communicator_utils.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/sync_utils.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/sync_errors.cpp

${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/ClockPropagationSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HCAAbstractClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HCAClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/JKClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/TwoLevelClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/offset/SKaMPIClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/offset/HCA3OffsetClockSync.cpp

${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/LinearModelFitterStandard.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/LinearModelFitterDebug.cpp

${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c

#${SRC_DIR}/reprompi_bench/option_parser/parse_options.c
#${SRC_DIR}/reprompi_bench/option_parser/parse_timing_options.c

${SRC_DIR}/log/zf_log.c
#${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c

#${SRC_DIR}/reprompi_bench/sync/time_measurement.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/sync_info.c
#${SRC_DIR}/reprompi_bench/misc.c
)

if(ENABLE_RDTSC)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_RDTSC")
    set(CLOCKSYNC_SRC_FILES
        ${CLOCKSYNC_SRC_FILES}
        ${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/RdtscClock.cpp
        )
endif()

if(ENABLE_RDTSCP)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_RDTSCP")
    set(CLOCKSYNC_SRC_FILES
            ${CLOCKSYNC_SRC_FILES}
            ${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/RdtscpClock.cpp
            )
endif()


set(REPROMPIBENCH_FILES
${SRC_DIR}/benchmarkMPIcalls.c
${SRC_DIR}/benchmarkCollective.c
${SRC_DIR}/benchmark_job.c
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

#find_library (synclib NAMES synclib)

ADD_SUBDIRECTORY(${SRC_DIR}/reprompi_bench/sync/clock_sync)

add_executable(mpibenchmark
${REPROMPIBENCH_FILES}
)
TARGET_LINK_LIBRARIES(mpibenchmark ${COMMON_LIBRARIES} synclib)
SET_TARGET_PROPERTIES(mpibenchmark PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
SET_TARGET_PROPERTIES(mpibenchmark PROPERTIES LINKER_LANGUAGE CXX )

if (ENABLE_PGMPI)
    if (PGMPI_FOUND)
        add_executable(mpibenchmark_pgmpicli
        ${REPROMPIBENCH_FILES}
        )
        TARGET_LINK_LIBRARIES(mpibenchmark_pgmpicli ${PGMPI_CLI_LIBRARIES} ${COMMON_LIBRARIES} synclib)
        SET_TARGET_PROPERTIES(mpibenchmark_pgmpicli PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
        SET_TARGET_PROPERTIES(mpibenchmark_pgmpicli PROPERTIES LINKER_LANGUAGE CXX )
        
        add_executable(mpibenchmark_pgmpituned
        ${REPROMPIBENCH_FILES}
        )
        TARGET_LINK_LIBRARIES(mpibenchmark_pgmpituned ${PGMPI_TUNED_LIBRARIES} ${COMMON_LIBRARIES} synclib)
        SET_TARGET_PROPERTIES(mpibenchmark_pgmpituned PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
        SET_TARGET_PROPERTIES(mpibenchmark_pgmpituned PROPERTIES LINKER_LANGUAGE CXX )
    endif(PGMPI_FOUND)

endif (ENABLE_PGMPI)
    
    
if (COMPILE_SANITY_CHECK_TESTS)
    ADD_SUBDIRECTORY(${SRC_DIR}/sanity_check)
endif()

if (COMPILE_BENCH_TESTS)
    ADD_SUBDIRECTORY(${SRC_DIR}/test)
endif()

if (COMPILE_SANITY_CHECK_CLOCK)
    ADD_SUBDIRECTORY(${SRC_DIR}/sanity_check/clock)
endif()

if (COMPILE_BENCH_LIBRARY)
    ADD_SUBDIRECTORY(${SRC_DIR}/reprompi_bench/benchmark_lib)
    ADD_SUBDIRECTORY(${SRC_DIR}/sanity_check/libtests)
endif()

if(OPTION_ENABLE_PGCHECKER)
    message(STATUS "Enabled: performance guidelines checker." )
    ADD_SUBDIRECTORY(${SRC_DIR}/pgcheck)
else()
    message(STATUS "Disabled: performance guidelines checker." )
endif()


set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_SOURCE_GENERATOR "TBZ2")


set(BENCHMARK_NAME ${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${BENCHMARK_NAME}-src")
set(DIST_ARCHIVE "${BENCHMARK_NAME}-dist")

SET(CPACK_SOURCE_IGNORE_FILES
"/CMakeFiles/"
"/_CPack_Packages/"
"/bin/"
"/include/"
"/lib/"
"/scripts/"
"/create_package.sh"
"/Makefile"
"\\\\.tar\\\\.bz2$"
"/\\\\.git/"
"/\\\\.settings/"
"/\\\\..*"
"CMakeCache.txt"
"cmake_install\\\\.cmake"
"CPackConfig\\\\.cmake"
"CPackSourceConfig\\\\.cmake"
"sources\\\\.org"
"${CPACK_SOURCE_IGNORE_FILES}"
)

INCLUDE(CPack)

add_custom_target(pack COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
