
#ifndef REPROMPI_BENCH_SYNC_PROCESS_SYNC_ROUND_SYNC_COMMON_H_
#define REPROMPI_BENCH_SYNC_PROCESS_SYNC_ROUND_SYNC_COMMON_H_

typedef struct {
  long bcast_n_rep;         /* --bcast-nrep how many bcasts to do for the mean */
  double bcast_multiplier;  /* --bcast-mult multiplier for mean Bcast time */
  int bcast_meas;           /* --bcast-meas how to compute the bcast run-time (mean, median, max) */
} reprompi_roundsync_bcast_params_t;

#define BCAST_MUTIPLIER 3
#define BCAST_NREP 10

enum {
  REPROMPI_ARGS_PROCSYNC_BCAST_MULTIPLIER = 1200,
  REPROMPI_ARGS_PROCSYNC_BCAST_NREP,
  REPROMPI_ARGS_PROCSYNC_BCAST_MEASURE
};

enum {
  BCAST_MEASURE_MEAN = 0,
  BCAST_MEASURE_MEDIAN,
  BCAST_MEASURE_MAX
};

double measure_bcast_runtime(MPI_Comm comm, reprompi_roundsync_bcast_params_t *parameters);
void roundsync_parse_bcast_options(int argc, char **argv, reprompi_roundsync_bcast_params_t* opts_p);

#endif /* REPROMPI_BENCH_SYNC_PROCESS_SYNC_ROUND_SYNC_COMMON_H_ */
