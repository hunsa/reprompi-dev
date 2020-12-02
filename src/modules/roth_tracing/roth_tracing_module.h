//
// Created by niklas on 05.11.20.
//

#ifndef REPROMPI_ROTH_TRACING_MODULE_H
#define REPROMPI_ROTH_TRACING_MODULE_H

#ifdef ROTH_TRACING

#include <roth_tracing/roth_tracing.h>

void print_roth_tracing(int my_rank, int procs, long nrep, int root);

#else

#define init_tracer(nrep)
#define roth_tracing_start_repetition(i)
#define roth_tracing_start_timer(time_metric)
#define roth_tracing_stop_timer(time_metric)
#define roth_tracing_end_repetition()
#define print_roth_tracing(my_rank, procs, nrep, root)

#endif //ROTH_TRACING

#endif //REPROMPI_ROTH_TRACING_MODULE_H
