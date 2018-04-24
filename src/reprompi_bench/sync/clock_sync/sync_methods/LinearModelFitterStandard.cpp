/*
 * LinearModelFitter.cpp
 *
 *  Created on: Apr 24, 2018
 *      Author: sascha
 */

#include <gsl/gsl_fit.h>

#include "LinearModelFitterStandard.hpp"

LinearModelFitterStandard::LinearModelFitterStandard() {

}


LinearModelFitterStandard::~LinearModelFitterStandard() {

}

int LinearModelFitterStandard::fit_linear_model(const double *xvals, const double *yvals, const int nb_vals,
    double *slope, double *intercept) {
  double cov00, cov01, cov11, sumsq;

  return gsl_fit_linear(xvals, 1, yvals, 1, nb_vals, intercept, slope, &cov00, &cov01, &cov11, &sumsq);
}
