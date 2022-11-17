
cmake_minimum_required(VERSION 3.22)

include(ExternalProject)

SET(PGTUNE_CMAKE_ARGS "")

find_package(GSL REQUIRED)
if (GSL_INCLUDE_DIR)
    message (STATUS "GSL INCLUDES: ${GSL_INCLUDE_DIR}")
else(GSL_INCLUDE_DIR)
    message (FATAL_ERROR "GSL libraries not found.")
endif(GSL_INCLUDE_DIR)

INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR} "src")
set(COMMON_LIBRARIES ${GSL_LIBRARIES} )

if(OPTION_ENABLE_LOGGING)
    SET(PGTUNE_CMAKE_ARGS "-DOPTION_ENABLE_LOGGING=ON ${PGTUNE_OPTIONS}")
    message(STATUS "Logging for pgtunelib enabled, cmake args: ${PGTUNE_CMAKE_ARGS}")
else()
    message(STATUS "Logging for pgtunelib disabled")
endif()

set(OFFLINE_BUILD_MODE off CACHE BOOL "download a fresh copy of all dependencies")

if (OFFLINE_BUILD_MODE)
    message(STATUS "disable git pull")
    ExternalProject_Add(pgtunelib
            GIT_REPOSITORY git@ophelia.par.tuwien.ac.at:hunold/pgtune_lib.git
            GIT_TAG pgcheck
            GIT_SUBMODULES submodules/mpi-lane-collectives
            CMAKE_ARGS ${PGTUNE_CMAKE_ARGS} -DOPTION_ENABLE_ALGID_STORING=OFF
            PREFIX ${CMAKE_SOURCE_DIR}/external/
            DOWNLOAD_COMMAND ""
            UPDATE_DISCONNECTED 1
    )
else()
    message(STATUS "enable git pull")
    ExternalProject_Add(pgtunelib
            GIT_REPOSITORY git@ophelia.par.tuwien.ac.at:hunold/pgtune_lib.git
            GIT_TAG pgcheck
            GIT_SUBMODULES submodules/mpi-lane-collectives
            CMAKE_ARGS ${PGTUNE_CMAKE_ARGS} -DOPTION_ENABLE_ALGID_STORING=OFF
            PREFIX ${CMAKE_SOURCE_DIR}/external/
    )
endif()


ExternalProject_Get_Property(pgtunelib install_dir)
set(PGTUNE_LIB_DIR ${install_dir}/src/pgtunelib-build/lib)
set(PGTUNE_INC_DIR ${install_dir}/src/pgtunelib-build/include)
LINK_DIRECTORIES(${PGTUNE_LIB_DIR})
INCLUDE_DIRECTORIES("${install_dir}/src/pgtunelib")

#execute_process(COMMAND ./external/src/pgtunelib-build/bin/pgmpi_info OUTPUT_VARIABLE PGMPI_INFO_OUTPUT)
#file(CONFIGURE OUTPUT "${install_dir}/src/pgtunelib-build/pgmpi_conf.csv"
#     CONTENT "${PGMPI_INFO_OUTPUT}")
set(PGTUNE_CONFIG_CSV ${install_dir}/src/pgtunelib-build/pgmpi_conf.csv)
add_custom_target(config
        COMMAND  bash -c "exec ${install_dir}/src/pgtunelib-build/bin/pgmpi_info > ${PGTUNE_CONFIG_CSV}"
        VERBATIM
)

add_executable(pgchecker
        pgchecker.cpp
        pgcheck_helper.cpp
        pgcheck_options.cpp
        pgtunelib_interface.cpp
        pgdata.cpp
        pgdata_table.cpp
        pgdata_comparer.cpp
        pgdata_printer.cpp
        comparer/comparer_data.cpp
        comparer/comparer_factory.cpp
        comparer/simple_comparer.cpp
        comparer/detailed_ttest_comparer.cpp
        comparer/ttest_comparer.cpp
        comparer/grouped_ttest_comparer.cpp
        pgcheck_input.cpp
        utils/string_utils.cpp
        utils/argv_manager.cpp
        utils/statistics_utils.cpp

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
SET_TARGET_PROPERTIES(pgchecker PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS} -I${PGTUNE_INC_DIR}")
TARGET_LINK_LIBRARIES(pgchecker ${COMMON_LIBRARIES} synclib pgmpicli)
SET_TARGET_PROPERTIES(pgchecker PROPERTIES LINK_FLAGS " -L${PGTUNE_LIB_DIR}")
SET_TARGET_PROPERTIES(pgchecker PROPERTIES LINKER_LANGUAGE CXX )



