/*
 * LinearModelFitterDebug.hpp
 *
 *  Created on: Apr 24, 2018
 *      Author: sascha
 */

#ifndef REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTERDEBUG_HPP_
#define REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTERDEBUG_HPP_


#include "LinearModelFitter.hpp"

class LinearModelFitterDebug : public LinearModelFitter {

private:
  MPI_Comm comm;
  int my_rank;
  int other_rank;
  const char *dbg_path;

public:
  LinearModelFitterDebug(MPI_Comm comm, int my_rank, int other_rank);
  ~LinearModelFitterDebug();

  int fit_linear_model(const double *xvals, const double *yvals, const int nb_vals, double *slope, double *intercept);

};


#endif /* REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTERDEBUG_HPP_ */
