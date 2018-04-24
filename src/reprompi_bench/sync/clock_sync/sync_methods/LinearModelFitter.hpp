/*
 * LinearModelFitter.hpp
 *
 *  Created on: Apr 24, 2018
 *      Author: sascha
 */

#ifndef REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTER_HPP_
#define REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTER_HPP_


class LinearModelFitter {

public:

  LinearModelFitter();
  virtual ~LinearModelFitter() = 0;

  virtual int fit_linear_model(const double *xvals, const double *yvals, const int nb_vals, double *slope,
      double *intercept) = 0;

};

inline LinearModelFitter::LinearModelFitter() { }
inline LinearModelFitter::~LinearModelFitter() { }

#endif /* REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_LINEARMODELFITTER_HPP_ */
