
add_executable(test_output
${SRC_DIR}/test/test_output/test_output_types.c
${SRC_DIR}/reprompi_bench/output_management/results_output.c
${SRC_DIR}/reprompi_bench/output_management/runtimes_computation.c

${SRC_DIR}/reprompi_bench/misc.c
${COMMON_OPTION_PARSER_SRC_FILES}
${SRC_DIR}/reprompi_bench/option_parser/parse_options.c
${SRC_DIR}/reprompi_bench/sync/barrier_sync/barrier_sync.c
${SRC_DIR}/reprompi_bench/sync/barrier_sync/dissem_barrier_sync.c
${COLL_OPS_SRC_FILES}
)


TARGET_LINK_LIBRARIES(test_output ${COMMON_LIBRARIES} )





