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
#include "benchmarkCollective.h"

#include "pgmpi_tune.h"

#include "pgcheck_helper.h"
#include "pgtunelib_interface.h"
#include "pgdata.h"
#include "pgdata_comparer.h"

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[]) {
  int rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  reprompib_register_sync_modules();
  reprompib_register_proc_sync_modules();
  reprompib_register_caching_modules();

  int nb_args = 5;
  int my_argc = nb_args+1;
  char **my_argv;

  my_argv = (char**)malloc(my_argc * sizeof(char*));
  for(int i=0; i<my_argc; i++) {
    my_argv[i] = (char*)malloc(100*sizeof(char));
  }

//  auto pginfo_data = exec_command("./external/src/pgtunelib-build/bin/pgmpi_info");

  auto csv_conf = "./external/src/pgtunelib-build/pgmpi_conf.csv";
  std::ifstream ifs(csv_conf);
  if(! ifs.good() ) {
    if( rank == 0 ) {
      std::cerr << "Cannot find " << csv_conf << std::endl;
    }
  }

  std::string pginfo_data( (std::istreambuf_iterator<char>(ifs) ),
                           (std::istreambuf_iterator<char>()    ) );

//  std::cout << "DATA\n" << pginfo_data << "\nENDDATA" << std::endl;
  auto pgtune_interface = PGTuneLibInterface(pginfo_data);

  for( auto& mpi_coll : pgtune_interface.get_available_mpi_collectives()) {
    PGDataComparer pgd_comparer(mpi_coll);
    std::cout << mpi_coll << std::endl;
    auto mod_name = pgtune_interface.get_module_name_for_mpi_collectives(mpi_coll);
    for( auto& alg_version : pgtune_interface.get_available_implementations_for_mpi_collectives(mpi_coll) ) {
      std::cout << mod_name << ":" << alg_version << std::endl;

      strcpy(my_argv[0], argv[0]);
      strcpy(my_argv[1], "--msizes-list=4,8");
      //strcpy(my_argv[1], "--msizes-list=4");
      strcpy(my_argv[2], ("--calls-list=" + mpi_coll).c_str());
      strcpy(my_argv[3], "--nrep=10");
      strcpy(my_argv[4], "--output-file=foo.txt");
      //strcpy(my_argv[5], ("--module=" + mod_name + "=" + "alg:" + alg_version).c_str());
      strcpy(my_argv[5], ("--module=" + mod_name + "=" + "alg:default").c_str());

      pgtune_override_argv_parameter(my_argc, my_argv);
      run_collective(my_argc, my_argv);
//
//      std::cout << rank << ": reading data" << std::endl;
//      if(rank == 0) {
//        auto *data = new PGData(mpi_coll, alg_version);
//        data->read_csv_from_file("foo.txt");
//        pgd_comparer.add_dataframe(alg_version, data);
//      }
    }

//    if(rank == 0) {
//      auto pgres = pgd_comparer.get_results();
//      std::cout << pgres << std::endl;
//    }

    break;
  }

  reprompib_deregister_sync_modules();
  reprompib_deregister_proc_sync_modules();
  reprompib_deregister_caching_modules();

  MPI_Finalize();

  return 0;
}
