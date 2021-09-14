


add_executable(benchtest1
benchtest1.c
#${SRC_DIR}/reprompi_bench/sync/time_measurement.c
)
SET_TARGET_PROPERTIES(benchtest1 PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(benchtest1 ${COMMON_LIBRARIES} reproMPIbench)



