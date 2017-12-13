#ifndef REPROMPIB_HCACLOCKSYNC_CLASS_H_
#define REPROMPIB_HCACLOCKSYNC_CLASS_H_

#include <math.h>
#include <mpi.h>
#include <gsl/gsl_fit.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffset.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "ClockSync.h"

typedef struct lm_s {
  double slope, intercept;
} lm_t;

template<class OffsetAlgType>
class HCA2ClockSync: public ClockSync {     // Hierarchical synchronization in log2(p) steps


private:
  int n_fitpoints; /* --fitpoints */
  int n_exchanges; /* --exchanges */
  OffsetAlgType* offset_algs;

  lm_t learn_model(const int root_rank, const int other_rank);
  lm_t merge_linear_models(lm_t lm1, lm_t lm2);

protected:
  int my_pow_2(int exp);


  virtual void compute_and_set_all_intercepts(lm_t* lm);
  virtual void compute_and_set_intercept(lm_t* lm, int client, int p_ref);

public:
  HCA2ClockSync(MPI_Comm comm, Clock* c);
  ~HCA2ClockSync();

  Clock* synchronize_all_clocks(void);

};

template<class OffsetAlgType>
inline HCA2ClockSync<OffsetAlgType>::HCA2ClockSync(MPI_Comm comm, Clock* c) :
    n_fitpoints(1000), n_exchanges(100), ClockSync(comm, c) {
  int nprocs, my_rank, i;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &nprocs);
  offset_algs = new OffsetAlgType[nprocs];
  for (i = 0; i < nprocs; i++) {
    offset_algs[i] = OffsetAlgType(i, my_rank, comm, n_exchanges);
  }
}

template<class OffsetAlgType>
inline HCA2ClockSync<OffsetAlgType>::~HCA2ClockSync() {
  delete[] offset_algs;
}

template<class OffsetAlgType>
inline lm_t HCA2ClockSync<OffsetAlgType>::learn_model(const int root_rank, const int other_rank) {
  int j;
  int my_rank;
  lm_t lm;

  lm.slope = 0;
  lm.intercept = 0;

  MPI_Comm_rank(this->comm, &my_rank);

  if (my_rank == root_rank) {
    for (j = 0; j < n_fitpoints; j++) {
      ClockOffset* offset = NULL;
      offset = offset_algs[other_rank].measure_offset(root_rank, *(this->local_clock));
      delete offset;
    }
  } else if (my_rank == other_rank) {
    double *xfit, *yfit;
    double cov00, cov01, cov11, sumsq;
    int fit;

    xfit = new double[n_fitpoints];
    yfit = new double[n_fitpoints];

    for (j = 0; j < n_fitpoints; j++) {
      ClockOffset* offset = NULL;

      offset = offset_algs[root_rank].measure_offset(root_rank, *(this->local_clock));
      xfit[j] = offset->get_timestamp();
      yfit[j] = offset->get_offset();

      delete offset;
    }

    fit = gsl_fit_linear(xfit, 1, yfit, 1, n_fitpoints, &lm.intercept, &lm.slope, &cov00, &cov01, &cov11, &sumsq);
    delete[] xfit;
    delete[] yfit;

  }
  return lm;
}

template<class OffsetAlgType>
inline lm_t HCA2ClockSync<OffsetAlgType>::merge_linear_models(lm_t lm1, lm_t lm2) {
  lm_t new_model;

  new_model.slope = lm1.slope + lm2.slope - lm1.slope * lm2.slope;
  new_model.intercept = lm1.intercept + lm2.intercept - lm2.intercept * lm1.slope;
  return new_model;
}

template<class OffsetAlgType>
inline int HCA2ClockSync<OffsetAlgType>::my_pow_2(int exp) {
  return (int) pow(2.0, (double) exp);
}


template<class OffsetAlgType>
inline void HCA2ClockSync<OffsetAlgType>::compute_and_set_intercept(lm_t* lm, int client, int p_ref)
{}

template<class OffsetAlgType>
inline void HCA2ClockSync<OffsetAlgType>::compute_and_set_all_intercepts(lm_t* lm)
{}

template<class OffsetAlgType>
inline Clock* HCA2ClockSync<OffsetAlgType>::synchronize_all_clocks(void) {
  int my_rank, nprocs;
  int i, j, p;

  int master_rank = 0;
  double *rtts_s;
  int n_pingpongs = 1000;
  int max_power_two, nrounds_step1;
  lm_t *linear_models, *tmp_linear_models;
  int running_power;
  int other_rank;
  double current_rtt;
  int nb_lm_to_comm;
  lm_t lm;

  MPI_Datatype dtype[2] = { MPI_DOUBLE, MPI_DOUBLE };
  int blocklen[2] = { 1, 1 };
  MPI_Aint disp[2] = { 0, sizeof(double) };
  MPI_Datatype mpi_lm_t;
  MPI_Status stat;

  int *step_two_group_ranks;
  int step_two_nb_ranks;
  MPI_Group orig_group, step_two_group;
  MPI_Comm step_two_comm;

  MPI_Comm_rank(this->comm, &my_rank);
  MPI_Comm_size(this->comm, &nprocs);

  nrounds_step1 = floor(log2((double) nprocs));
  max_power_two = (int) pow(2.0, (double) nrounds_step1);

  linear_models = new lm_t[nprocs];
  tmp_linear_models = new lm_t[nprocs];

  MPI_Type_create_struct(2, blocklen, disp, dtype, &mpi_lm_t);
  MPI_Type_commit(&mpi_lm_t);

  for (i = 0; i < nrounds_step1; i++) {

    running_power = my_pow_2(i + 1);

    // get a client/server pair
    // : estimate rtt
    // : learn clock model
    // : gather clock model
    // : adjust model
    if (my_rank >= max_power_two) {
      //
    } else {
      if (my_rank % running_power == 0) {

        // master
        other_rank = my_rank + my_pow_2(i);

        // compute model through linear regression
        learn_model(my_rank, other_rank);
        compute_and_set_intercept(NULL, other_rank, my_rank);

        // so I also need to receive some other models from my partner rank
        // there should be 2^i models to receive
        nb_lm_to_comm = my_pow_2(i);

        if (nb_lm_to_comm > 0) {
          MPI_Recv(&tmp_linear_models[0], nb_lm_to_comm, mpi_lm_t, other_rank, 0, this->comm, &stat);

          linear_models[other_rank] = tmp_linear_models[0];
          for (j = 1; j < nb_lm_to_comm; j++) {
            linear_models[other_rank + j] = merge_linear_models(linear_models[other_rank], tmp_linear_models[j]);
          }
        }

      } else if (my_rank % running_power == my_pow_2(i)) {
        // client
        other_rank = my_rank - my_pow_2(i);

        // compute model through linear regression
        linear_models[my_rank] = learn_model(other_rank, my_rank);

        compute_and_set_intercept(&linear_models[my_rank], my_rank, other_rank);

        // I will need to send my models back to the master
        // there should be 2^i models to send
        // starting from my rank
        nb_lm_to_comm = my_pow_2(i);

        if (nb_lm_to_comm > 0) {
          MPI_Send(&linear_models[my_rank], nb_lm_to_comm, mpi_lm_t, other_rank, 0, this->comm);
        }
      }
    }
    MPI_Barrier(this->comm);
  }

  // now step 2
  // synchronize processes with ranks > 2^max_power_two
  if (nprocs > max_power_two) {
    MPI_Comm_group(this->comm, &orig_group);
    step_two_nb_ranks = nprocs - max_power_two + 1;
    step_two_group_ranks = new int[step_two_nb_ranks];
    step_two_group_ranks[0] = master_rank;
    j = 1;
    for (p = max_power_two; p < nprocs; p++) {
      step_two_group_ranks[j] = p;
      j++;
    }

    MPI_Group_incl(orig_group, step_two_nb_ranks, step_two_group_ranks, &step_two_group);

    MPI_Comm_create(this->comm, step_two_group, &step_two_comm);

    if (my_rank < max_power_two) {
      if (my_rank + max_power_two < nprocs) {
        other_rank = my_rank + max_power_two;

        // compute model through linear regression
        linear_models[other_rank] = learn_model(my_rank, other_rank);
        compute_and_set_intercept(NULL, other_rank, my_rank);
      }

    } else {
      other_rank = my_rank - max_power_two;

      // compute model through linear regression
      linear_models[my_rank] = learn_model(other_rank, my_rank);
      compute_and_set_intercept(&linear_models[my_rank], my_rank, other_rank);
    }

    if (step_two_comm != MPI_COMM_NULL) {
      // 0 in sub comm is master rank
      MPI_Gather(&linear_models[my_rank], 1, mpi_lm_t, &tmp_linear_models[0], 1, mpi_lm_t, 0, step_two_comm);
      MPI_Comm_free(&step_two_comm);
    }

    if (my_rank == master_rank) {
      // max_power_two ranks start in buffer at position 1
      for (j = 1; j < step_two_nb_ranks; j++) {
        // now we have the time between
        // p and p-max_power_two
        p = max_power_two - 1 + j;
        if (j == 1) { // master rank
          linear_models[p].slope = tmp_linear_models[j].slope;
          linear_models[p].intercept = tmp_linear_models[j].intercept;
        } else {
          linear_models[p] = merge_linear_models(linear_models[j - 1], tmp_linear_models[j]);
        }
      }
    }

    delete[] step_two_group_ranks;
  }

  MPI_Scatter(linear_models, 1, mpi_lm_t, &lm, 1, mpi_lm_t, master_rank, this->comm);
  compute_and_set_all_intercepts(&lm);
  MPI_Barrier(this->comm);

  if (my_rank == master_rank) {
    lm.slope = 0;
    lm.intercept = 0;
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &master_rank);
  //printf("[rank %d] rank=%d s=%f i=%f nprocs=%d\n",master_rank, my_rank, lm.slope, lm.intercept,nprocs );
  return new GlobalClockLM(this->local_clock, lm.slope, lm.intercept);
}

#endif /*  REPROMPIB_HCACLOCKSYNC_CLASS_H_  */
