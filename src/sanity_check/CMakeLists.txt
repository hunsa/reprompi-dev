
add_executable(compare_clocks
compare_clocks.c
parse_test_options.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
)

add_executable(measure_rtt
measure_rtt.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
)

set(MEASURE_CLOCK_DRIFT
clock_drift/measure_clock_drift.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
)

add_executable(measure_clock_drift
    ${MEASURE_CLOCK_DRIFT}
)

add_executable(measure_clock_drift_new
clock_drift/measure_clock_drift_new.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
#${SRC_DIR}/reprompi_bench/sync/process_sync/process_synchronization.c
#${SRC_DIR}/reprompi_bench/sync/process_sync/barrier_sync.c
#${SRC_DIR}/reprompi_bench/sync/process_sync/round_time_sync.c
#${SRC_DIR}/reprompi_bench/sync/process_sync/window_sync.c
#${SRC_DIR}/reprompi_bench/sync/process_sync/round_sync_common.c
)


add_executable(measure_real_clock_drift
clock_drift/measure_real_clock_drift.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
)

set(SYNC_DURATION
sync_duration.c
parse_test_options.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
${SYNC_SRC_FILES}
)

add_executable(sync_duration
    ${SYNC_DURATION}
)


SET_TARGET_PROPERTIES(measure_clock_drift PROPERTIES LINKER_LANGUAGE CXX )
SET_TARGET_PROPERTIES(measure_clock_drift PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(measure_clock_drift ${COMMON_LIBRARIES} synclib)

SET_TARGET_PROPERTIES(measure_clock_drift_new PROPERTIES LINKER_LANGUAGE CXX )
SET_TARGET_PROPERTIES(measure_clock_drift_new PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(measure_clock_drift_new ${COMMON_LIBRARIES} synclib)

SET_TARGET_PROPERTIES(measure_real_clock_drift PROPERTIES LINKER_LANGUAGE CXX )
SET_TARGET_PROPERTIES(measure_real_clock_drift PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(measure_real_clock_drift ${COMMON_LIBRARIES} synclib)

TARGET_LINK_LIBRARIES(sync_duration ${COMMON_LIBRARIES} synclib)
SET_TARGET_PROPERTIES(sync_duration PROPERTIES LINKER_LANGUAGE CXX )
SET_TARGET_PROPERTIES(sync_duration PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")

TARGET_LINK_LIBRARIES(compare_clocks ${COMMON_LIBRARIES} )
SET_TARGET_PROPERTIES(compare_clocks PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")

TARGET_LINK_LIBRARIES(measure_rtt ${COMMON_LIBRARIES} )
SET_TARGET_PROPERTIES(measure_rtt PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")





