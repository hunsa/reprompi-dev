


add_executable(verify_clock
verify_clock.c
)
SET_TARGET_PROPERTIES(verify_clock PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(verify_clock MPITS::mpits MPI::MPI_C)



