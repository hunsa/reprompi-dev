/*
 * LinearModelFitter.cpp
 *
 *  Created on: Apr 24, 2018
 *      Author: sascha
 */

#include <mpi.h>
#include <gsl/gsl_fit.h>

#include <iostream>
#include <cstdlib>
#include <fstream>

#include "LinearModelFitterDebug.hpp"

#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
//#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

LinearModelFitterDebug::LinearModelFitterDebug(MPI_Comm comm, int my_rank, int other_rank) :
comm (comm), my_rank(my_rank), other_rank(other_rank)
{
}


LinearModelFitterDebug::~LinearModelFitterDebug() {

}

int LinearModelFitterDebug::fit_linear_model(const double *xvals, const double *yvals, const int nb_vals,
    double *slope, double *intercept) {
  double cov00, cov01, cov11, sumsq;

//  ZF_LOGV("%d-%d (global %d-%d)", my_rank, other_rank, my_rank_world, other_rank_world);

  const char *dbg_path = std::getenv("REPROMPI_DEBUG_PATH");
  if( dbg_path == NULL ) {
    ZF_LOGW("REPROMPI_DEBUG_PATH not set");
  } else {
    std::ofstream myfile;
    //std::string abs_path = dbg_path + "/" + "lin_model_" + my_rank_world + "_" + other_rank_world;
    char abs_path[100];

    int my_rank_world, other_rank_world;

    MPI_Group my_group, world_group;

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Comm_group(comm, &my_group);

    MPI_Group_translate_ranks(my_group, 1, &my_rank, world_group, &my_rank_world);
    MPI_Group_translate_ranks(my_group, 1, &other_rank, world_group, &other_rank_world);


    sprintf(abs_path, "%s/lin_model_%d_%d.res", dbg_path, my_rank_world, other_rank_world);

    myfile.open(abs_path);
    for(int i=0; i<nb_vals; i++) {
      myfile << xvals[i] << ";" << yvals[i] << std::endl;
    }
    myfile.close();
  }

  return gsl_fit_linear(xvals, 1, yvals, 1, nb_vals, intercept, slope, &cov00, &cov01, &cov11, &sumsq);
}
