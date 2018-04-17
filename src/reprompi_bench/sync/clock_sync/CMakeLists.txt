

set(CLOCKSYNC_SRC_FILES
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClock.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/MPIClock.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/RdtscpClock.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/RdtscClock.cpp

${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffset.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.cpp

${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/hwloc_helpers.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/communicator_utils.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/sync_utils.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/hca_sync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sk_sync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/jk_sync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/topo_aware_sync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clock_sync_common.cpp
${SRC_DIR}/reprompi_bench/sync/common/sync_module_helpers.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/synchronization.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/no_sync.c

${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HierarchicalClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/JKClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/SKaMPIClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/ClockPropagationSync.cpp

#${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
#${SRC_DIR}/reprompi_bench/option_parser/parse_options.c
#${SRC_DIR}/reprompi_bench/option_parser/parse_timing_options.c

${SRC_DIR}/log/zf_log.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
#${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c

#${SRC_DIR}/reprompi_bench/sync/time_measurement.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/sync_info.c
#${SRC_DIR}/reprompi_bench/misc.c
)


add_library(synclib
${CLOCKSYNC_SRC_FILES}
)
TARGET_LINK_LIBRARIES(synclib ${COMMON_LIBRARIES} ${HWLOC_LIBRARIES})
SET_TARGET_PROPERTIES(synclib PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")


