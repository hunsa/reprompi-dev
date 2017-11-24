
add_executable(test_output
${SRC_DIR}/test/test_output/test_output_types.c
${SRC_DIR}/reprompi_bench/output_management/results_output.c
${SRC_DIR}/reprompi_bench/output_management/runtimes_computation.c

${SRC_DIR}/reprompi_bench/misc.c
${COMMON_OPTION_PARSER_SRC_FILES}
${SRC_DIR}/reprompi_bench/option_parser/parse_options.c
${COLL_OPS_SRC_FILES}
)


TARGET_LINK_LIBRARIES(test_output ${COMMON_LIBRARIES} )





