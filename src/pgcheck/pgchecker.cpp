//
// Created by Sascha on 11/26/21.
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <numeric>
#include "mpi.h"
#include <sys/stat.h>

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/process_sync/process_synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "benchmark_job.h"
#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/option_parser/parse_timing_options.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/output_management/runtimes_computation.h"
#include "reprompi_bench/output_management/results_output.h"
#include "collective_ops/collectives.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "reprompi_bench/caching/caching.h"
#include "reprompi_bench/sync/clock_sync/utils/communicator_utils.h"
#include "benchmarkCollective.h"

#include "pgmpi_tune.h"

#include "pgcheck_helper.h"
#include "pgcheck_options.h"
#include "pgtunelib_interface.h"
#include "pgdata.h"
#include "pgdata_printer.h"
#include "pgdata_comparer.h"
#include "comparer/simple_comparer.h"
#include "comparer/ttest_comparer.h"
#include "comparer/grouped_ttest_comparer.h"
#include "comparer/comparer_factory.h"
#include "pgcheck_input.h"
#include "utils/argv_manager.h"
#include "utils/statistics_utils.h"

static double get_barrier_runtime() {
  double avg_runtime = -1.0;
  int rank;
  // run a barrier test
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto barrier_options = "--calls-list=MPI_Barrier --msizes-list=1 --nrep=500 --proc-sync=roundtime --rt-bench-time-ms=2000 --bcast-nrep=1 --rt-barrier-count=0 --output-file=barrier.txt";
  std::vector <std::string> barrier_argv_vector;
  auto foo = std::vector<std::string>();
  argv::compose_argv_vector("dummy", barrier_options, foo, barrier_argv_vector);
  int argc_test = 0;
  char **argv_test;

  argv::convert_vector_to_argv_cstyle(barrier_argv_vector, &argc_test, &argv_test);
  pgtune_override_argv_parameter(argc_test, argv_test);
  run_collective(argc_test, argv_test);
  argv::free_argv_cstyle(argc_test, argv_test);

  if (rank == 0) {
    auto data = new PGData("MPI_Barrier", "default");
    data->read_csv_from_file("barrier.txt");
    auto vals = data->get_runtimes_for_count(1);
    //double sum = std::accumulate(vals.begin(), vals.end(), 0.0);
    //avg_runtime = sum / vals.size();
    avg_runtime = StatisticsUtils<double>().median(vals);
  }
  MPI_Bcast(&avg_runtime, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  return avg_runtime;
}

int main(int argc, char *argv[]) {
  int rank;
  int ppn, nnodes, size;
  MPI_Comm comm_intranode;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  create_intranode_communicator(MPI_COMM_WORLD, &comm_intranode);
  MPI_Comm_size(comm_intranode, &ppn);
  nnodes = size / ppn;

  PGCheckOptions *options = new PGCheckOptions();
  PGDataPrinter *printer = NULL;
  std::string parse_res = options->parse(argc, argv);

  if (rank == 0) {
    printer = new PGDataPrinter(options);
  }

  // -h, --help was specified, terminate success
  if (parse_res == "h") {
    if (rank == 0) {
      printer->print_usage(argv[0]);
    }
    MPI_Comm_free(&comm_intranode);
    MPI_Finalize();
    exit(EXIT_SUCCESS);
  }
  // error parsing options, terminate failure
  else if (parse_res == "e"){
    if (rank == 0) {
      printer->println_error_to_cerr("cannot parse options");
      printer->print_usage(argv[0]);
    }
    exit(EXIT_FAILURE);
  }
  // print warnings
  else if (parse_res != "") {
    if (rank == 0) {
      printer->println_warning_to_cout(parse_res);
    }
  }

  reprompib_register_sync_modules();
  reprompib_register_proc_sync_modules();
  reprompib_register_caching_modules();

  std::ifstream ins(options->get_input_file());
  if (!ins.good()) {
    if (rank == 0) {
      printer->println_error_to_cerr("input file '" + options->get_input_file() + "' cannot be read");
      printer->print_usage(argv[0]);
      fflush(stdout);
      exit(-1);
    }
  }

  double barrier_mean = get_barrier_runtime();
  if (rank == 0) {
    printer->println_to_cout("mean barrier: " + std::to_string(barrier_mean));
  }

  PGInput input(options->get_input_file());

  std::string csv_conf = "./external/src/pgtunelib-build/pgmpi_conf.csv";
  std::ifstream ifs(csv_conf);
  if (!ifs.good()) {
    if (rank == 0) {
      printer->println_error_to_cerr("cannot find '" + csv_conf + "'");
    }
    exit(-1);
  }

  std::string pginfo_data((std::istreambuf_iterator<char>(ifs)),
                          (std::istreambuf_iterator<char>()));

  auto pgtune_interface = PGTuneLibInterface(pginfo_data);

  for (int case_id = 0; case_id < input.get_number_of_test_cases(); case_id++) {
    std::string mpi_coll = input.get_mpi_collective_for_case_id(case_id);

    if (rank == 0) {
      printer->println_to_cout("Case " + std::to_string(case_id) + ": " + mpi_coll);
    }

    auto mod_name = pgtune_interface.get_module_name_for_mpi_collectives(mpi_coll);
    std::unordered_map < std::string, PGData * > coll_data;
    for (auto &alg_version: pgtune_interface.get_available_implementations_for_mpi_collectives(mpi_coll)) {
      std::vector <std::string> pgtunelib_argv;
      if (rank == 0) {
        printer->println_to_cout(mod_name + ":" + alg_version);
      }

      std::string call_options = input.get_call_options_for_case_id(case_id);
      pgtunelib_argv.push_back("--calls-list=" + mpi_coll);
      pgtunelib_argv.push_back("--output-file=foo.txt");
      pgtunelib_argv.push_back("--module=" + mod_name + "=" + "alg:" + alg_version);
      std::vector <std::string> argv_vector;
      argv::compose_argv_vector(argv[0], call_options, pgtunelib_argv, argv_vector);

      int argc_test = 0;
      char **argv_test;
      argv::convert_vector_to_argv_cstyle(argv_vector, &argc_test, &argv_test);

      pgtune_override_argv_parameter(argc_test, argv_test);
      run_collective(argc_test, argv_test);
      argv::free_argv_cstyle(argc_test, argv_test);

      if (rank == 0) {
        auto *data = new PGData(mpi_coll, alg_version);
        data->read_csv_from_file("foo.txt");
        coll_data.insert({alg_version, data});
      }
    }

    if (rank == 0) {
      PGDataComparer *comparer = ComparerFactory::create_comparer(options->get_comparer_type(), mpi_coll, nnodes, ppn);
      comparer->set_barrier_time(barrier_mean);
      comparer->add_data(coll_data);
      if (printer->print_collective(comparer) != 0) {
        printer->println_error_to_cerr("cannot print results");
        exit(-1);
      }
    }
  }

  if (rank == 0) {
    printer->print_summary();
    delete printer;
  }

  reprompib_deregister_sync_modules();
  reprompib_deregister_proc_sync_modules();
  reprompib_deregister_caching_modules();

  MPI_Comm_free(&comm_intranode);

  MPI_Finalize();

  return 0;
}
