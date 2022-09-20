
SET(SYNC_LIB_DEP_LIBS "")

# in case that we have HWLOC, we include the sync method based on HWLOC
if (HWLOC_INCLUDE_DIR)

set(CLOCKSYNC_SRC_FILES
${CLOCKSYNC_SRC_FILES}
${SRC_DIR}/reprompi_bench/sync/clock_sync/topo_aware_sync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/sync_methods/HierarchicalClockSync.cpp
${SRC_DIR}/reprompi_bench/sync/clock_sync/utils/hwloc_helpers.cpp
)

set(SYNC_LIB_DEP_LIBS
${SYNC_LIB_DEP_LIBS}
${HWLOC_LIBRARIES}
)

endif (HWLOC_INCLUDE_DIR)

add_library(synclib
${CLOCKSYNC_SRC_FILES}
)
TARGET_LINK_LIBRARIES(synclib ${COMMON_LIBRARIES} ${SYNC_LIB_DEP_LIBS})
SET_TARGET_PROPERTIES(synclib PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")

install(TARGETS synclib DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

