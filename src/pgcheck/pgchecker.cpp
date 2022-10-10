//
// Created by Sascha on 11/26/21.
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include "mpi.h"

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
#include "pgtunelib_interface.h"
#include "pgdata.h"
#include "pgdata_comparer.h"
#include "pgcheck_input.h"
#include "utils/argv_manager.h"

#include <getopt.h>
#include <iostream>
#include <fstream>
#include <string>

static std::string input_file = "";

void print_usage(char *command) {
  std::cout << "usage: " << std::string(command) << " -f [input_file]" << std::endl;
}

void parse_options(int argc, char *argv[]) {
  int c;
  while ((c = getopt(argc, argv, "hf:")) != -1)
  {
    switch (c) {
    case '?':
    case 'h':
    default :
      print_usage(argv[0]);
      exit(-1);
    case 'f':
      input_file = std::string(optarg);
      break;
    }
  }
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

  parse_options(argc, argv);

  reprompib_register_sync_modules();
  reprompib_register_proc_sync_modules();
  reprompib_register_caching_modules();

  std::ifstream ins(input_file);
  if( !ins.good() ) {
    if( rank == 0 ) {
      std::cerr << "Error: input file '" << input_file << "' cannot be read\n" << std::endl;
      print_usage(argv[0]);
      fflush(stdout);
      exit(-1);
    }
  }

  PGInput input(input_file);

//  int nb_args = 5;
//  int my_argc = nb_args+1;
//  char **my_argv;
//
//  my_argv = (char**)malloc(my_argc * sizeof(char*));
//  for(int i=0; i<my_argc; i++) {
//    my_argv[i] = (char*)malloc(100*sizeof(char));
//  }

  //auto pginfo_data = exec_command("./external/src/pgtunelib-build/bin/pgmpi_info");

  auto csv_conf = "./external/src/pgtunelib-build/pgmpi_conf.csv";
  std::ifstream ifs(csv_conf);
  if(! ifs.good() ) {
    if( rank == 0 ) {
      std::cerr << "Cannot find " << csv_conf << std::endl;
    }
    exit(-1);
  }

  std::string pginfo_data( (std::istreambuf_iterator<char>(ifs) ),
                           (std::istreambuf_iterator<char>()    ) );

//  std::cout << "DATA\n" << pginfo_data << "\nENDDATA" << std::endl;
  auto pgtune_interface = PGTuneLibInterface(pginfo_data);

  for(int case_id=0; case_id<input.get_number_of_test_cases(); case_id++) {
    std::string mpi_coll = input.get_mpi_collective_for_case_id(case_id);
    if( rank == 0 ) {
      std::cout << "Case " << case_id << ": " << mpi_coll << std::endl;
    }
    PGDataComparer pgd_comparer(mpi_coll, nnodes, ppn);
    auto mod_name = pgtune_interface.get_module_name_for_mpi_collectives(mpi_coll);
    for( auto& alg_version : pgtune_interface.get_available_implementations_for_mpi_collectives(mpi_coll) ) {
      std::vector<std::string> pgtunelib_argv;
      if( rank == 0 ) {
        std::cout << mod_name << ":" << alg_version << std::endl;
      }


//      strcpy(my_argv[0], argv[0]);
//      strcpy(my_argv[1], "--msizes-list=4,8");
//      strcpy(my_argv[2], ("--calls-list=" + mpi_coll).c_str());
//      strcpy(my_argv[3], "--nrep=10");
//      strcpy(my_argv[4], "--output-file=foo.txt");
//      strcpy(my_argv[5], ("--module=" + mod_name + "=" + "alg:" + alg_version).c_str());
      //strcpy(my_argv[5], ("--module=" + mod_name + "=" + "alg:default").c_str());

      std::string call_options = input.get_call_options_for_case_id(case_id);
      pgtunelib_argv.push_back("--calls-list=" + mpi_coll);
      pgtunelib_argv.push_back("--output-file=foo.txt");
      pgtunelib_argv.push_back("--module=" + mod_name + "=" + "alg:" + alg_version);
      std::vector<std::string> argv_vector;
      argv::compose_argv_vector(argv[0], call_options, pgtunelib_argv, argv_vector);

//      for(auto it=argv_vector.begin(); it!=argv_vector.end(); it++) {
//        std::cout << "argv_part" << ":" << *it << std::endl;
//      }

      int argc_test=0;
      char **argv_test;
      argv::convert_vector_to_argv_cstyle(argv_vector, &argc_test, &argv_test);

//      for(int i=0; i<argc_test; i++) {
//        std::cout << "argv_part" << ":" << argv_test[i] << std::endl;
//      }

//      pgtune_override_argv_parameter(my_argc, my_argv);
//      run_collective(my_argc, my_argv);
      pgtune_override_argv_parameter(argc_test, argv_test);
      run_collective(argc_test, argv_test);

      argv::free_argv_cstyle(argc_test, argv_test);

//      std::cout << rank << ": reading data" << std::endl;
      if(rank == 0) {
        auto *data = new PGData(mpi_coll, alg_version);
        data->read_csv_from_file("foo.txt");
        pgd_comparer.add_dataframe(alg_version, data);
      }
    }

    if(rank == 0) {
      auto pgres = pgd_comparer.get_results_t_test();
      std::cout << pgres << std::endl;
    }
    //break;
  }

  reprompib_deregister_sync_modules();
  reprompib_deregister_proc_sync_modules();
  reprompib_deregister_caching_modules();

  MPI_Comm_free(&comm_intranode);

  MPI_Finalize();

  return 0;
}
