/*
 * HCA2ClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <math.h>
#include <gsl/gsl_fit.h>

#include "HCA2ClockSync.h"
#include "LinearModelFitterStandard.hpp"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

HCA2ClockSync::HCA2ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, bool recompute_intercept) :
  HCAClockSync(offsetAlg, n_fitpoints), recompute_intercept(recompute_intercept) {
}

HCA2ClockSync::~HCA2ClockSync() {
}

void HCA2ClockSync::remeasure_intercept_call_back(MPI_Comm comm, Clock &c, LinModel* lm, int client, int p_ref) {
  ZF_LOGV("compute intercept call back in HCA2");

  if (true == this->recompute_intercept) {
    remeasure_intercept(comm, c, lm, client, p_ref);
  }

}

void HCA2ClockSync::remeasure_all_intercepts_call_back(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank) {
  ZF_LOGV("compute all intercepts call back in HCA2");
  // empty call back here
  // nothing to be done for HCA2
}
