

#include <math.h>
#include <gsl/gsl_fit.h>

#include "HCAAbstractClockSync.h"
#include "LinearModelFitterStandard.hpp"


#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
//#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

HCAAbstractClockSync::HCAAbstractClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints) :
    offset_alg(offsetAlg), n_fitpoints(n_fitpoints) {
}

HCAAbstractClockSync::~HCAAbstractClockSync() {
}


/**
 * re-computes the offset (intercept) after learning one model after each round
 *
 * this is the actual implementation
 * it may or may not be called by the "call_back" which is virtual in every subclass
 *
 */
void HCAAbstractClockSync::remeasure_intercept(MPI_Comm comm, Clock &c, LinModel* lm, int client, int p_ref) {
  ZF_LOGV("compute intercept in HCAAbstractClockSync");
  int my_rank;

  MPI_Comm_rank(comm, &my_rank);

  if (my_rank == p_ref) {
    ClockOffset* offset = NULL;
    offset = offset_alg->measure_offset(comm, p_ref, client, c);
    delete offset;
  } else if (my_rank == client) {
    ClockOffset* offset = NULL;
    offset = offset_alg->measure_offset(comm, p_ref, client, c);
    lm->intercept = (lm->slope) * (-offset->get_timestamp()) + offset->get_offset();
    delete offset;
  }
}

/**
 * re-computes the offset (intercept) after learning ALL models
 *
 * same as compute_and_set_intercept (also real implementation)
 *
 */
void HCAAbstractClockSync::remeasure_all_intercepts(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank) {
  int my_rank, np;

  ZF_LOGV("compute all intercepts in HCAAbstractClockSync");

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &np);

  if (my_rank != ref_rank) {
    remeasure_intercept(comm, c, lm, my_rank, ref_rank);
  } else {
    for (int i = 0; i < np; i++) {
      if (i != ref_rank) {
        remeasure_intercept(comm, c, lm, i, ref_rank);
      }
    }
  }
}


LinModel HCAAbstractClockSync::learn_model(MPI_Comm comm, Clock &c, const int root_rank, const int other_rank) {
  int j;
  int my_rank;
  LinModel lm;


  lm.slope = 0;
  lm.intercept = 0;

  MPI_Comm_rank(comm, &my_rank);

  if (my_rank == root_rank) {
    for (j = 0; j < n_fitpoints; j++) {
      ClockOffset* offset = NULL;
      offset = offset_alg->measure_offset(comm, root_rank, other_rank, c);
      delete offset;
    }

    remeasure_intercept_call_back(comm, c, NULL, other_rank, root_rank);

  } else if (my_rank == other_rank) {
    double *xfit, *yfit;
    // double cov00, cov01, cov11, sumsq;
    int fit;
    LinearModelFitter *fitter = new LinearModelFitterStandard();

    xfit = new double[n_fitpoints];
    yfit = new double[n_fitpoints];

    for (j = 0; j < n_fitpoints; j++) {
      ClockOffset* offset = NULL;

      offset = offset_alg->measure_offset(comm, root_rank, my_rank, c);

      xfit[j] = offset->get_timestamp();
      yfit[j] = offset->get_offset();

      delete offset;
    }

//    fit = gsl_fit_linear(xfit, 1, yfit, 1, n_fitpoints, &lm.intercept, &lm.slope, &cov00, &cov01, &cov11, &sumsq);
    fit = fitter->fit_linear_model(xfit, yfit, n_fitpoints, &lm.slope, &lm.intercept);

    delete[] xfit;
    delete[] yfit;
    delete fitter;


    remeasure_intercept_call_back(comm, c, &lm, other_rank, root_rank);
  }



  return lm;
}

LinModel HCAAbstractClockSync::merge_linear_models(LinModel lm1, LinModel lm2) {
  LinModel new_model;

  new_model.slope = lm1.slope + lm2.slope - lm1.slope * lm2.slope;
  new_model.intercept = lm1.intercept + lm2.intercept - lm2.intercept * lm1.slope;
  return new_model;
}

int HCAAbstractClockSync::my_pow_2(int exp) {
  return (int) pow(2.0, (double) exp);
}


