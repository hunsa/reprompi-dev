

set(CLOCKSYNC_SRC_FILES
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClock.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.cpp

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

${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HierarchicalClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/JKClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/SKaMPIClockSync.cpp

${SRC_DIR}/log/zf_log.c

#${SRC_DIR}/reprompi_bench/sync/time_measurement.c
#${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/sync_info.c
#${SRC_DIR}/reprompi_bench/misc.c
)


add_library(synclib
${CLOCKSYNC_SRC_FILES}
)
TARGET_LINK_LIBRARIES(synclib ${COMMON_LIBRARIES} ${HWLOC_LIBRARIES})
SET_TARGET_PROPERTIES(synclib PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")


