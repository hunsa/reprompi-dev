/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
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
#include <stddef.h>
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

#ifdef PAPI

#include "papi.h"

#else

#define PAPI_OK 0
#define PAPI_strerror(retVal) ""
#define PAPI_hl_region_begin(region) PAPI_OK
#define PAPI_hl_region_end(region) PAPI_OK
#define PAPI_hl_stop() PAPI_OK

#endif

#ifdef LIKWID_PERFMON

#include <likwid.h>

#else
#define LIKWID_MARKER_INIT
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_SWITCH
#define LIKWID_MARKER_REGISTER(regionTag)
#define LIKWID_MARKER_START(regionTag)
#define LIKWID_MARKER_STOP(regionTag)
#define LIKWID_MARKER_CLOSE
#define LIKWID_MARKER_GET(regionTag, nevents, events, time, count)
#endif

#define MY_MAX(x, y) (((x) > (y)) ? (x) : (y))

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

typedef struct pvar_record {
    int rank;
    long nrep;
    char pvar_name[256];
    long long value;
} pvar_record;

static const int OUTPUT_ROOT_PROC = 0;

char *get_region_name(long rep, long nrep);

void handle_error(int retval);

void get_all_MPI_T_pvars(char ***pvars, int *num);

void get_handles_for_MPI_T_pvars(char **pvar_names, MPI_T_pvar_handle *pvar_handles, int number_of_pvars,
                                 MPI_T_pvar_session session);

void print_pvars(int my_rank, int procs, const pvar_record *pvar_records, int number_of_pvar_records);

static void print_initial_settings(const reprompib_options_t *opts, const reprompib_common_options_t *common_opts,
        //const reprompib_dictionary_t* dict,
                                   const reprompib_bench_print_info_t *print_info) {
    int my_rank, np;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    print_common_settings(print_info, common_opts);//, dict);

    if (my_rank == OUTPUT_ROOT_PROC) {
        FILE *f;

        f = stdout;
        if (opts->n_rep > 0) {
            fprintf(f, "#@nrep=%ld\n", opts->n_rep);
            if (common_opts->output_file != NULL) {
                f = fopen(common_opts->output_file, "a");
                fprintf(f, "#@nrep=%ld\n", opts->n_rep);
                fflush(f);
                fclose(f);
            }
        }
    }
}


static void reprompib_print_bench_output(job_t job, double *tstart_sec, double *tend_sec,
                                         const reprompib_options_t *opts, const reprompib_common_options_t *common_opts,
                                         const reprompib_bench_print_info_t *print_info) {

    FILE *f = stdout;
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (common_opts->output_file != NULL) {
            f = fopen(common_opts->output_file, "a");
        }
    }

    if (opts->print_summary_methods > 0) {
        print_summary(stdout, job, tstart_sec, tend_sec, print_info, opts);
        if (common_opts->output_file != NULL) {
            print_measurement_results(f, job, tstart_sec, tend_sec, print_info, opts);
        }

    } else {
        print_measurement_results(f, job, tstart_sec, tend_sec, print_info, opts);
    }

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (common_opts->output_file != NULL) {
            fflush(f);
            fclose(f);
        }
    }

}


static void reprompib_parse_bench_options(int argc, char **argv) {
    int c;
    opterr = 0;

    const struct option bench_long_options[] = {
            {"help", required_argument, 0, 'h'},
            {0, 0,                      0, 0}
    };

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "h", bench_long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 'h': /* list of summary options */
                reprompib_print_benchmark_help();
                break;
            case '?':
                break;
        }
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options
}


int main(int argc, char *argv[]) {
    //printf("# Started\n");
    int my_rank, procs;
    long i, jindex;
    double *tstart_sec;
    double *tend_sec;
    reprompib_options_t opts;
    reprompib_common_options_t common_opts;
    job_list_t jlist;
    collective_params_t coll_params;
    basic_collective_params_t coll_basic_info;
    time_t start_time, end_time;
    reprompib_bench_print_info_t print_info;

    reprompib_sync_module_t clock_sync;
    reprompib_proc_sync_module_t proc_sync;
    reprompib_sync_params_t sync_params;
    reprompib_timing_method_t runtime_type;
    reprompi_caching_module_t caching_module;
    //long total_n_rep;
    int is_invalid;

    //PAPI return value
    int retval;

    /* start up MPI
     *
     * */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    // init MPI_T
    int provided;
    MPI_T_init_thread(MPI_THREAD_SINGLE, &provided);
    char **pvar_names;
    int number_of_pvars;
    get_all_MPI_T_pvars(&pvar_names, &number_of_pvars);
    /*char *pvar_names[] = {"runtime_spc_OMPI_SPC_BYTES_RECEIVED_USER",
                          "runtime_spc_OMPI_SPC_BYTES_RECEIVED_MPI",
                          "runtime_spc_OMPI_SPC_BYTES_SENT_USER",
                          "runtime_spc_OMPI_SPC_BYTES_SENT_MPI"};
    int number_of_pvars = NELEMS(pvar_names);*/
    MPI_T_pvar_handle pvar_handles[number_of_pvars];
    long long int pvar_prev_values[number_of_pvars];

    MPI_T_pvar_session session;
    MPI_T_pvar_session_create(&session);

    // printf("Session created\n");
    // fflush(stdout);

//    if (my_rank == 0) {
//        for (int j = 0; j < number_of_pvars; j++) {
//            printf("%d ", j);
//            fflush(stdout);
//            printf("(%X -> ", &pvar_names[j]);
//            fflush(stdout);
//            printf("%X): ", pvar_names[j]);
//            fflush(stdout);
//            printf("%s\n", pvar_names[j]);
//            fflush(stdout);
//        }
//    }

    get_handles_for_MPI_T_pvars(pvar_names, pvar_handles, number_of_pvars, session);

    for (int j = 0; j < number_of_pvars; j++) {
        // printf("# Index for pvar %s: %s\n", pvar_names[j], pvar_handles[j]->pvar->name);
        MPI_T_pvar_start(session, pvar_handles[j]);
    }
    // printf("Rank %d: handles initialized\n", my_rank);

    // init LIKWID
    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;

    reprompib_register_sync_modules();
    reprompib_register_proc_sync_modules();

    reprompib_register_caching_modules();

    // initialize time measurement functions
    init_timer();
    start_time = time(NULL);


    // parse arguments and set-up benchmarking jobs
    print_command_line_args(argc, argv);

    reprompib_parse_bench_options(argc, argv);  // only "-h" for help

    // parse common arguments (e.g., msizes list, MPI calls to benchmark, input file)
    reprompib_parse_common_options(&common_opts, argc, argv);

    // parse timing options
    reprompib_parse_timing_options(&runtime_type, argc, argv);

    // parse the benchmark-specific arguments (nreps, summary)
    reprompib_parse_options(&opts, argc, argv);

    // INIT LIKWID Sections
    for (i = 0; i < opts.n_rep; i++) {
        char *region_name = get_region_name(i, opts.n_rep);
        LIKWID_MARKER_REGISTER(region_name);
        free(region_name);
    }

    // INIT pvar record array
    int number_of_pvar_records = (int) opts.n_rep * number_of_pvars;
    pvar_record *pvar_records = malloc(number_of_pvar_records * sizeof(pvar_record));

    // initialize caching strategies
    reprompib_init_caching_module(argc, argv, &caching_module);

    // initialize synchronization module
    reprompib_init_sync_module(argc, argv, &clock_sync);
    reprompib_init_proc_sync_module(argc, argv, &clock_sync, &proc_sync);

    if (common_opts.input_file == NULL && opts.n_rep <= 0) { // make sure nrep is specified when there is no input file
        reprompib_print_error_and_exit(
                "The number of repetitions is not defined (specify the \"--nrep\" command-line argument or provide an input file)\n");
    }
    generate_job_list(&common_opts, opts.n_rep, &jlist);


    init_collective_basic_info(common_opts, procs, &coll_basic_info);
    // execute the benchmark jobs
    for (jindex = 0; jindex < jlist.n_jobs; jindex++) {
        job_t job;
        job = jlist.jobs[jlist.job_indices[jindex]];

        tstart_sec = (double *) malloc(job.n_rep * sizeof(double));
        tend_sec = (double *) malloc(job.n_rep * sizeof(double));

        collective_calls[job.call_index].initialize_data(coll_basic_info, job.count, &coll_params);

        // initialize synchronization
        sync_params.nrep = job.n_rep;
        sync_params.count = job.count;
        proc_sync.init_sync(&sync_params);
        clock_sync.init_sync();


        print_info.clock_sync = &clock_sync;
        print_info.proc_sync = &proc_sync;
        print_info.timing_method = runtime_type;
        if (jindex == 0) {
            //print_initial_settings(&opts, &common_opts, &params_dict, &print_info);
            print_initial_settings(&opts, &common_opts, &print_info);
            print_results_header(&print_info, &opts, common_opts.output_file, opts.verbose);
        }

        clock_sync.sync_clocks();
        proc_sync.init_sync_round();         // broadcast first window


        i = 0;
        while (1) {
            //Reset pvar Counters
            for (int j = 0; j < number_of_pvars; j++) {
                long long int value;
                MPI_T_pvar_reset(session, pvar_handles[j]);
                MPI_T_pvar_read(session, pvar_handles[j], &value);
                pvar_prev_values[j] = value;
            }

            //Start LIKWID Region
            char *regionTag = get_region_name(i, job.n_rep);
            LIKWID_MARKER_START(regionTag);

            //Start PAPI Region
            retval = PAPI_hl_region_begin(regionTag);
            if (retval != PAPI_OK) {
                handle_error(retval);
            }

            proc_sync.start_sync();

            tstart_sec[i] = get_time();
            collective_calls[job.call_index].collective_call(&coll_params);
            tend_sec[i] = get_time();

            is_invalid = proc_sync.stop_sync();

            // End PAPI region
            retval = PAPI_hl_region_end(regionTag);
            if (retval != PAPI_OK) {
                handle_error(retval);
            }

            // End LIKWID region
            LIKWID_MARKER_STOP(regionTag);

            // Save pvar variables
            //printf("Rank %d: saving pvars\n", my_rank);
            for (int j = 0; j < number_of_pvars; j++) {
                pvar_record pvar_record;
                pvar_record.nrep = i;
                pvar_record.rank = my_rank;
                strcpy(pvar_record.pvar_name, pvar_names[j]);
                long long int value;
                MPI_T_pvar_read(session, pvar_handles[j], &value);
                pvar_record.value = value - pvar_prev_values[j];
                pvar_records[i * number_of_pvars + j] = pvar_record;
            }
            // printf("Rank %d: saved pvars %ld/%d, nrep: %ld/%ld\n", my_rank, i * number_of_pvars,
            //        number_of_pvar_records, i, opts.n_rep);
            // fflush(stdout);

            free(regionTag);
            if (is_invalid == REPROMPI_INVALID_MEASUREMENT) {
                // redo the measurement
                // we are still in the time frame
                //ZF_LOGV("[%d] invalid_measurement at i=%ld", my_rank, total_n_rep);
            } else if (is_invalid == REPROMPI_OUT_OF_TIME_VALID) {
                job.n_rep = i + 1;
                break;
            } else if (is_invalid == REPROMPI_OUT_OF_TIME_INVALID) {
                job.n_rep = MY_MAX(0, i - 1);
                break;
            } else {
                i++;
            }
            if (i == job.n_rep) {
                break;
            }


            // apply cache cleaning strategy (if enabled)
            caching_module.clear_cache();
        }
        //print summarized data
        reprompib_print_bench_output(job, tstart_sec, tend_sec, &opts, &common_opts, &print_info);

        //print pvar data
        print_pvars(my_rank, procs, pvar_records, number_of_pvar_records);


        clock_sync.finalize_sync();
        proc_sync.finalize_sync();

        free(tstart_sec);
        free(tend_sec);

        collective_calls[job.call_index].cleanup_data(&coll_params);
    }


    end_time = time(NULL);

    // Clean up LIKWID
    LIKWID_MARKER_CLOSE;

    //Clean up PAPI
    retval = PAPI_hl_stop();
    if (retval != PAPI_OK) {
        handle_error(retval);
    }
    print_final_info(&common_opts, start_time, end_time);
    fflush(stdout);

    //Clean up MPI_T
    for (int j = 0; j < number_of_pvars; j++) {
        // printf("Rank %d freeing pvar handle %d\n", my_rank, j);
        // fflush(stdout);
        MPI_T_pvar_stop(session, pvar_handles[j]);
        // MPI_T_pvar_handle_free(session, &pvar_handles[j]);
        // printf("Rank %d freeing pvar name %d (%X->%s)\n", my_rank, j, &pvar_names[j], pvar_names[j]);
        // fflush(stdout);
        // free(pvar_names[j]);
    }
    // printf("Rank %d freeing pvar names\n", my_rank);
    // fflush(stdout);
    // free(pvar_names);
    // printf("Rank %d freeing pvar records\n", my_rank);
    // fflush(stdout);
    // free(pvar_records);
    MPI_T_pvar_session_free(&session);
    MPI_T_finalize();

    cleanup_job_list(jlist);
    reprompib_free_common_parameters(&common_opts);
    reprompib_free_parameters(&opts);
//    reprompib_cleanup_dictionary(&params_dict);
    clock_sync.cleanup_module();
    proc_sync.cleanup_module();

    caching_module.cleanup_module();

    reprompib_deregister_sync_modules();
    reprompib_deregister_proc_sync_modules();
    reprompib_deregister_caching_modules();
    /* shut down MPI */
    MPI_Finalize();

    return 0;
}

void print_pvars(int my_rank, int procs, const pvar_record *pvar_records, int number_of_pvar_records) {
    int root = 0;
    MPI_Datatype mpi_pvar_record;
    int array_of_block_lengths[] = {1, 1, 256, 1};
    MPI_Aint array_of_displacements[] = {
            offsetof(pvar_record, rank),
            offsetof(pvar_record, nrep),
            offsetof(pvar_record, pvar_name),
            offsetof(pvar_record, value)
    };
    MPI_Datatype array_of_types[] = {MPI_INT, MPI_LONG, MPI_CHAR, MPI_LONG_LONG};

    MPI_Type_create_struct(4, array_of_block_lengths, array_of_displacements, array_of_types, &mpi_pvar_record);
    MPI_Type_commit(&mpi_pvar_record);

    pvar_record *pvars_receive_buffer;
    if (my_rank == root) {
        pvars_receive_buffer = malloc(procs * number_of_pvar_records * sizeof(pvar_record));
    }
    MPI_Gather(pvar_records, number_of_pvar_records, mpi_pvar_record,
               pvars_receive_buffer, number_of_pvar_records, mpi_pvar_record,
               root, MPI_COMM_WORLD);
    if (my_rank == root) {
        printf("# Number of pvar_records: %d\n", number_of_pvar_records * procs);
        printf("nrep,rank,pvar_name,value\n");
        for (int j = 0; j < number_of_pvar_records * procs; j++) {
            printf("%ld,%d,%s,%lld\n",
                   pvars_receive_buffer[j].nrep,
                   pvars_receive_buffer[j].rank,
                   pvars_receive_buffer[j].pvar_name,
                   pvars_receive_buffer[j].value);
            fflush(stdout);
        }
    }
}

char *get_region_name(long rep, long nrep) {
    //printf("Number of jobs %ld\n", nrep);
    int maxDigits;
    if (nrep > 1) {
        maxDigits = (int) log10((double) nrep - 1) + 1;
    } else {
        maxDigits = 1;
    }
    int tagLength = 6 + maxDigits;
    //printf("Tag length %d\n", tagLength);
    char *regionTag;
    regionTag = malloc(sizeof(char) * tagLength);
    snprintf(regionTag, tagLength, "nrep_%0*ld", maxDigits, rep);
    //printf("Started region %s\n", regionTag);
    return regionTag;
}

void handle_error(int retval) {
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    exit(1);
}

void get_all_MPI_T_pvars(char ***pvars, int *num) {
    int i, name_len, desc_len, verbosity, bind, var_class, readonly, continuous, atomic, rc;
    MPI_Datatype datatype;
    MPI_T_enum enumtype;
    char name[256], description[256];

    MPI_T_pvar_get_num(num);
    *pvars = malloc(sizeof(char *) * *num);
    int v = 0;
    for (i = 0; i < *num; i++) {
        name_len = desc_len = 256;
        rc = PMPI_T_pvar_get_info(i, name, &name_len, &verbosity,
                                  &var_class, &datatype, &enumtype, description, &desc_len, &bind,
                                  &readonly, &continuous, &atomic);
        if (rc == MPI_SUCCESS) {
            (*pvars)[v] = malloc(sizeof(char) * (name_len + 1));
            strncpy((*pvars)[v], name, name_len + 1);
            v++;
        }
    }
    // *pvars = realloc(*pvars, sizeof(char *) * v);
    *num = v;
}

void get_handles_for_MPI_T_pvars(char **pvar_names, MPI_T_pvar_handle *pvar_handles, int number_of_pvars,
                                 MPI_T_pvar_session session) {
    int i, num, name_len, desc_len, verbosity, bind, var_class, readonly, continuous, atomic, count, index;
    MPI_Datatype datatype;
    MPI_T_enum enumtype;
    char name[256], description[256];

    for (int v = 0; v < number_of_pvars; v++) {
        index = -1;
        MPI_T_pvar_get_num(&num);
        // printf("Creating handle for pvar nr. %d\n", v);
        // fflush(stdout);
        for (i = 0; i < num; i++) {
            name_len = desc_len = 256;
            int rc = PMPI_T_pvar_get_info(i, name, &name_len, &verbosity,
                                          &var_class, &datatype, &enumtype, description, &desc_len, &bind,
                                          &readonly, &continuous, &atomic);
            if (MPI_SUCCESS != rc) {
                continue;
            }
            // printf("Comparing %d (%s) and %d (%s)\n", v, pvar_name, i, name);
            // fflush(stdout);
            if (strncmp(name, pvar_names[v], name_len + 1) == 0) {
                index = i;
                break;
            }
        }

        /* Make sure we found the counters */
        if (index == -1) {
            fprintf(stderr, "ERROR: Couldn't find the appropriate SPC counter %s in the MPI_T pvars.\n", pvar_names[v]);
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        // printf("Init handles %d\n", v);
        // fflush(stdout);
        MPI_T_pvar_handle handle;
        /* Create the MPI_T sessions/handles for the counters and start the counters */
        MPI_T_pvar_handle_alloc(session, index, NULL, &handle, &count);
        MPI_T_pvar_start(session, handle);

        // printf("Created handle for pvar nr. %d (%s)\n", v, name);
        // fflush(stdout);
        pvar_handles[v] = handle;
    }
}

