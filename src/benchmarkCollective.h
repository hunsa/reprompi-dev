//
// Created by Sascha on 9/23/22.
//

#ifndef REPROMPI_SRC_BENCHMARKCOLLECTIVE_H
#define REPROMPI_SRC_BENCHMARKCOLLECTIVE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This is just the function that is called by mpiBenchmark and the pgchecker.
 * It cannot be called alone, as the modules need to be registered first.
 *
 * @param argc
 * @param argv
 */
void run_collective(int argc, char **argv, mpits_clocksync_t *clock_sync);

#ifdef __cplusplus
}
#endif


#endif //REPROMPI_SRC_BENCHMARKCOLLECTIVE_H
