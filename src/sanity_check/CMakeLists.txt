
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
measure_clock_drift.c
parse_test_options.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
${SYNC_SRC_FILES}
)

add_executable(measure_clock_drift
    ${MEASURE_CLOCK_DRIFT}
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


TARGET_LINK_LIBRARIES(measure_clock_drift ${COMMON_LIBRARIES} synclib)
TARGET_LINK_LIBRARIES(sync_duration ${COMMON_LIBRARIES} synclib)

TARGET_LINK_LIBRARIES(compare_clocks ${COMMON_LIBRARIES} )
TARGET_LINK_LIBRARIES(measure_rtt ${COMMON_LIBRARIES} )


SET_TARGET_PROPERTIES(measure_clock_drift PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
SET_TARGET_PROPERTIES(sync_duration PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")


SET_TARGET_PROPERTIES(compare_clocks PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
SET_TARGET_PROPERTIES(measure_rtt PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")





