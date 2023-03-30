/*  PGChecker - MPI Performance Guidelines Checker
 *
 *  Copyright 2023 Sascha Hunold, Maximilian Hagn
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <numeric>
#include <chrono>

#include "mpi.h"
#include "pgmpi_tune.h"
#include "constants.h"

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

#include "pgtunelib_interface.h"
#include "pgcheck_helper.h"
#include "pgcheck_options.h"
#include "pgcheck_input.h"
#include "pgdata.h"
#include "pgdata_printer.h"
#include "pgdata_comparer.h"
#include "comparer/simple_comparer.h"
#include "comparer/violation/violation_comparer.h"
#include "comparer/violation/grouped_violation_comparer.h"
#include "comparer/comparer_factory.h"
#include "utils/argv_manager.h"
#include "utils/statistics_utils.h"

constexpr auto ROOT_PROCESS_RANK = 0;

std::string get_runtime_string(int64_t nanoseconds) {
  std::ostringstream stream;
  stream << "PGChecker Runtime:      ";
  if (nanoseconds >= 60000000000) {
    int64_t minutes = nanoseconds / 60000000000;
    int64_t seconds = (nanoseconds - (minutes * 60000000000)) / 1000000000;
    int64_t remainingNanoseconds =
        nanoseconds - (minutes * 60000000000) - (seconds * 1000000000);
    stream << minutes << " min " << seconds << " s "
           << remainingNanoseconds / 1000000 << " ms";
  } else if (nanoseconds >= 1000000000) {
    int64_t seconds = nanoseconds / 1000000000;
    int64_t remainingNanoseconds = nanoseconds - (seconds * 1000000000);
    stream << seconds << " s " << remainingNanoseconds / 1000000 << " ms";
  } else if (nanoseconds >= 1000000) {
    int64_t milliseconds = nanoseconds / 1000000;
    int64_t remainingNanoseconds = nanoseconds - (milliseconds * 1000000);
    stream << milliseconds << " ms " << remainingNanoseconds / 1000 << " µs";
  } else if (nanoseconds >= 1000) {
    int64_t microseconds = nanoseconds / 1000;
    int64_t remainingNanoseconds = nanoseconds - (microseconds * 1000);
    stream << microseconds << " µs " << remainingNanoseconds << " ns";
  } else {
    stream << nanoseconds << " ns";
  }
  return stream.str();
}

static double get_barrier_runtime() {
  double avg_runtime = -1.0;
  int rank;
  // run a barrier test
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto barrier_options =
      "--calls-list=MPI_Barrier --msizes-list=1 --nrep=500 --proc-sync=roundtime "
      "--rt-bench-time-ms=2000 --bcast-nrep=1 --rt-barrier-count=0 --output-file=barrier.txt";
  std::vector <std::string> barrier_argv_vector;
  auto foo = std::vector<std::string>();
  argv::compose_argv_vector("dummy", barrier_options, foo, barrier_argv_vector);
  int argc_test = 0;
  char **argv_test;

  argv::convert_vector_to_argv_cstyle(barrier_argv_vector,
                                      &argc_test,
                                      &argv_test);
  pgtune_override_argv_parameter(argc_test, argv_test);
  run_collective(argc_test, argv_test);
  argv::free_argv_cstyle(argc_test, argv_test);

  if (rank == 0) {
    auto data = new PGData("MPI_Barrier", "default");
    data->read_csv_from_file("barrier.txt");
    avg_runtime = StatisticsUtils<double>().median(
        data->get_runtimes_for_count(1));
  }
  MPI_Bcast(&avg_runtime, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  return avg_runtime;
}

int main(int argc, char *argv[]) {
  int rank, ppn, node_count, comm_size;
  MPI_Comm comm_intranode;
  std::string tmp_dir = "./data";

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  create_intranode_communicator(MPI_COMM_WORLD, &comm_intranode);
  MPI_Comm_size(comm_intranode, &ppn);
  node_count = comm_size / ppn;

  PGCheckOptions options;
  PGDataPrinter *printer = NULL;
  int64_t pg_checker_runtime = 0;

  if (rank == ROOT_PROCESS_RANK) {
    printer = new PGDataPrinter();
  }

  if (!options.parse(argc, argv)) {
    if (rank == ROOT_PROCESS_RANK) {
      printer->print_usage(argv[0]);
    }

    MPI_Comm_free(&comm_intranode);
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  if (rank == ROOT_PROCESS_RANK) {
    printer->set_options(options);
  }

  reprompib_register_sync_modules();
  reprompib_register_proc_sync_modules();
  reprompib_register_caching_modules();

  std::ifstream ins(options.get_input_file());
  if (!ins.good()) {
    if (rank == ROOT_PROCESS_RANK) {
      printer->println_error_to_cerr(
          "input file '" + options.get_input_file() + "' cannot be read");
      printer->print_usage(argv[0]);
      fflush(stdout);
      return EXIT_FAILURE;
    }
  }

  double barrier_mean = get_barrier_runtime();
  if (rank == ROOT_PROCESS_RANK) {
    printer->println_to_cout(
        "avg barrier [ms]: " + std::to_string(barrier_mean * 1000.0));
  }

  PGInput input(options.get_input_file());

  std::string csv_conf = "./external/src/pgtunelib-build/pgmpi_conf.csv";
  std::ifstream ifs(csv_conf);
  if (!ifs.good()) {
    if (rank == ROOT_PROCESS_RANK) {
      printer->println_error_to_cerr("cannot find '" + csv_conf + "'");
    }
    return EXIT_FAILURE;
  }

  std::string pginfo_data((std::istreambuf_iterator<char>(ifs)),
                          (std::istreambuf_iterator<char>()));

  auto pgtune_interface = PGTuneLibInterface(pginfo_data);

  size_t merge_table_id = 0;

  for (int case_id = 0; case_id < input.get_number_of_test_cases(); case_id++) {
    std::string mpi_coll = input.get_mpi_collective_for_case_id(case_id);

    if (rank == ROOT_PROCESS_RANK) {
      if (options.get_verbose()) {
        printer->println_info_to_cout(
            "Case " + std::to_string(case_id) + ": " + mpi_coll);
      }
    }

    auto mod_name = pgtune_interface.get_module_name_for_mpi_collectives(
        mpi_coll);
    std::unordered_map < std::string, PGData * > coll_data;
    for (auto &alg_version : pgtune_interface.get_available_implementations_for_mpi_collectives(mpi_coll)) {
      std::vector <std::string> pgtunelib_argv;
      if (rank == ROOT_PROCESS_RANK) {
        printer->println_info_to_cout(
            "Collecting Data:        " + mod_name + ":" + alg_version);
      }

      std::string call_options = input.get_call_options_for_case_id(case_id);
      std::string tmp_out_file =
          tmp_dir + "/" + "data_" + mpi_coll + "_" + alg_version + ".dat";
      if (rank == ROOT_PROCESS_RANK) {
        struct stat sb;
        stat(tmp_dir.c_str(), &sb);
        if (!S_ISDIR(sb.st_mode)) {
          mkdir(tmp_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
      }

      pgtunelib_argv.push_back("--calls-list=" + mpi_coll);
      pgtunelib_argv.push_back("--output-file=" + tmp_out_file);
      pgtunelib_argv.push_back(
          "--module=" + mod_name + "=" + "alg:" + alg_version);
      std::vector <std::string> argv_vector;
      argv::compose_argv_vector(argv[0], call_options, pgtunelib_argv,
                                argv_vector);

      int argc_test = 0;
      char **argv_test;
      argv::convert_vector_to_argv_cstyle(argv_vector, &argc_test, &argv_test);

      pgtune_override_argv_parameter(argc_test, argv_test);
      if (rank == ROOT_PROCESS_RANK) {
        if (options.get_verbose()) {
          std::cout << "\033[34m" << "[INFO]    " << "\033[0m";
          print_command_line_args(argc_test, argv_test);
        }
      }

      run_collective(argc_test, argv_test);
      argv::free_argv_cstyle(argc_test, argv_test);

      if (rank == ROOT_PROCESS_RANK) {
        auto *data = new PGData(mpi_coll, alg_version);
        data->read_csv_from_file(tmp_out_file);
        coll_data.insert({alg_version, data});
      }
    }

    if (rank == ROOT_PROCESS_RANK) {
      auto runtime_start = std::chrono::high_resolution_clock::now();
      printer->println_info_to_cout("Collecting Finished:    " + mod_name);
      for (auto comparer_type : options.get_comparer_list()) {
        printer->println_info_to_cout(
            "Evaluating Data:        comparer:" +
            CONSTANTS::COMPARER_NAMES.at(comparer_type));
        PGDataComparer *comparer = ComparerFactory::create_comparer(
            comparer_type, options.get_test_type(), mpi_coll,
            node_count, ppn);
        comparer->set_barrier_time(barrier_mean);
        comparer->add_data(coll_data);
        if (printer->print_collective(comparer, comparer_type,
                                      merge_table_id++) != 0) {
          printer->println_error_to_cerr("cannot print results");
          exit(-1);
        }
      }
      auto runtime_end = std::chrono::high_resolution_clock::now();
      pg_checker_runtime += std::chrono::duration_cast<std::chrono::nanoseconds>(
          runtime_end - runtime_start).count();
    }
    merge_table_id = 0;
  }

  if (rank == ROOT_PROCESS_RANK) {
    auto runtime_start = std::chrono::high_resolution_clock::now();
    printer->print_summary();
    auto runtime_end = std::chrono::high_resolution_clock::now();
    pg_checker_runtime += std::chrono::duration_cast<std::chrono::milliseconds>(
        runtime_end - runtime_start).count();
    printer->println_info_to_cout(get_runtime_string(pg_checker_runtime));
    delete printer;
  }

  reprompib_deregister_sync_modules();
  reprompib_deregister_proc_sync_modules();
  reprompib_deregister_caching_modules();

  MPI_Comm_free(&comm_intranode);

  MPI_Finalize();

  return EXIT_SUCCESS;
}
