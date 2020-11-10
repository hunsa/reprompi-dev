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
#include <dirent.h>
#include <getopt.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
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

#include <modules/utils.h>
#include "modules/roth_tracing/roth_tracing_module.h"
#include "modules/papi/papi_module.h"
#include "modules/likwid/likwid_module.h"
#include "modules/MPI_T_pvar/MPI_T_pvar_module.h"


#define MY_MAX(x, y) (((x) > (y)) ? (x) : (y))

typedef struct process_record {
    int rank;
    char hostname[64];
    long nrep;
    long pid;
    char process_name[256];
    unsigned long time;
} process_record;

static const int OUTPUT_ROOT_PROC = 0;

int get_pids(long **pids);

void save_process_prev_times(long num_pids, const long *pids, unsigned long *process_prev_times);

int get_info_from_pid(long pid, int *name_length, char *name, unsigned long *time);

void
save_process_records(long nrep, int my_rank, int num_pids, long *pids, const unsigned long *process_prev_times,
                     process_record *process_records);

void
print_process_records(int my_rank, int procs, const process_record *process_records, int number_of_process_records);


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
            {0,      0,                 0, 0}
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

    // init pids
    long *pids;
    int num_pids = get_pids(&pids);

    // for (int j = 0; j < num_pids; j++) {
    //     printf("# rank_%d: %ld\n", my_rank, pids[j]);
    // }
    unsigned long process_prev_times[num_pids];

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
    initialize_likwid_regions(opts.n_rep);

    // init MPI_T
    init_MPI_T_pvars(opts.n_rep);

    // INIT process record array
    int number_of_process_records = (int) opts.n_rep * num_pids;
    process_record *process_records = malloc(number_of_process_records * sizeof(process_record));

    // INIT microbenchmark array
    init_tracer(opts.n_rep, (long) my_rank);

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
            //Remember process prev time
            //save_process_prev_times(num_pids, pids, process_prev_times);

            //Reset pvar Counters
            start_MPI_T_pvars();

            //Start LIKWID Region
            char *regionTag = get_region_name(i, job.n_rep);
            LIKWID_MARKER_START(regionTag);

            //Start PAPI Region
            retval = PAPI_hl_region_begin(regionTag);
            if (retval != PAPI_OK) {
                handle_error(retval);
            }

            proc_sync.start_sync();
            roth_tracing_start_repetition(i);

            roth_tracing_start_timer(REPROMPI_RUNTIME_OUTER);
            tstart_sec[i] = get_time();
            roth_tracing_start_timer(REPROMPI_RUNTIME_INNER);
            collective_calls[job.call_index].collective_call(&coll_params);
            roth_tracing_stop_timer(REPROMPI_RUNTIME_INNER);
            tend_sec[i] = get_time();
            roth_tracing_stop_timer(REPROMPI_RUNTIME_OUTER);
            roth_tracing_end_repetition();

            is_invalid = proc_sync.stop_sync();

            // End PAPI region
            retval = PAPI_hl_region_end(regionTag);
            if (retval != PAPI_OK) {
                handle_error(retval);
            }

            // End LIKWID region
            LIKWID_MARKER_STOP(regionTag);


            // Save pvar variables
            save_MPI_T_pvars(i, my_rank);

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
        print_pvars(my_rank, OUTPUT_ROOT_PROC, procs);

        print_roth_tracing(my_rank, procs, opts.n_rep, OUTPUT_ROOT_PROC);

        //print process data
        //print_process_records(my_rank, procs, process_records, number_of_process_records);


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
    clean_up_MPI_T_pvars();

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

bool isNumeric(char *string) {
    // Get value with failure detection.

    char *next;
    long val = strtol(string, &next, 10);

    // Check for empty string and characters left after conversion.

    if ((next == string) || (*next != '\0')) {
        return false;
    } else {
        return true;
    }
}

int get_pids(long **pids) {
    *pids = NULL;
    int pid_cnt = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (isNumeric(dir->d_name)) {
                pid_cnt++;
                *pids = realloc(*pids, sizeof(long) * pid_cnt);
                char *ignore = NULL;
                (*pids)[pid_cnt - 1] = strtol(dir->d_name, &ignore, 10);
                //printf("# pid_%d: %ld\n", pid_cnt-1, (*pids)[pid_cnt - 1]);
            }
        }
        closedir(d);
    }
    return pid_cnt;
}

void save_process_prev_times(long num_pids, const long *pids, unsigned long *process_prev_times) {
    for (int j = 0; j < num_pids; j++) {
        unsigned long time;
        long pid = pids[j];
        char name[256];
        int name_length;
        get_info_from_pid(pid, &name_length, name, &time);
        process_prev_times[j] = time;
    }
}

void save_process_records(long nrep, int my_rank, int num_pids, long *pids, const unsigned long *process_prev_times,
                          process_record *process_records) {
    for (int i = 0; i < num_pids; i++) {
        // printf("# Saving pid_number: %d\n", i);
        //printf("## PIDs: ");
        //for (int j = 0; j < num_pids; j++) {
        //printf("%ld, ", pids[j]);
        //}
        //printf("\n");
        fflush(stdout);
        long pid = pids[i];
        //printf("## Saving pid: %ld\n", pid);
        //fflush(stdout);
        unsigned long time;
        int name_length;
        char name[256];
        get_info_from_pid(pid, &name_length, name, &time);
        //printf("## Name: %s\n", name);
        //fflush(stdout);
        time -= process_prev_times[i];

        long index = num_pids * nrep + i;
        process_records[index].time = time;
        process_records[index].pid = pid;
        process_records[index].rank = my_rank;
        process_records[index].nrep = nrep;
        strncpy(process_records[index].process_name, name, name_length + 1);
        gethostname(process_records[index].hostname, 64);
        // printf("## #%ld Rank_%d: %s (%ld): %lu\n", nrep, my_rank, process_records[index].process_name, process_records[index].pid, process_records[index].time);
        fflush(stdout);
    }
}

int get_info_from_pid(long pid, int *name_length, char *name, unsigned long *time) {
    FILE *fp;
    char stat_file_name[20];
    snprintf(stat_file_name, 20, "/proc/%ld/stat", pid);
    //printf("## stat_file: %s\n", stat_file_name);
    //fflush(stdout);
    //fp = fopen(stat_file_name, "r");
    //if (fp == NULL) {
    //    fprintf(stderr, "Couldn't open file %s, Error %d: %s\n", stat_file_name, errno, strerror(errno));
    //    return errno;
    //}
    int buffer_size = 1024;
    char buffer[buffer_size];
    strncpy(buffer,
            "1 (systemd) S 0 1 1 0 -1 4194560 109962 2745609 95 1915 1334 6360 6260 5589 20 0 1 0 13 172765184 3135 18446744073709551615 1 1 0 0 0 0 671173123 4096 1260 0 0 0 17 0 0 0 215 0 0 0 0 0 0 0 0 0 0",
            buffer_size);
    // fgets(buffer, buffer_size, fp);
    // fclose(fp);
    int first_parenthesis_open = -1;
    int last_parenthesis_closed = -1;
    for (int k = 0; k < buffer_size && buffer[k] != '\0'; k++) {
        if (buffer[k] == ')') {
            last_parenthesis_closed = k;
        } else if (first_parenthesis_open < 0 && buffer[k] == '(') {
            first_parenthesis_open = k;
        }
    }
    *name_length = last_parenthesis_closed - first_parenthesis_open - 1;
    char format[256];
    sprintf(format,
            "%%*ld (%%%d[^\n] ) %%c %%*d %%*d %%*d %%*d %%*d %%*u %%*lu %%*lu %%*lu %%*lu %%lu %%lu %%lu %%lu",
            *name_length);
    unsigned long utime, stime, cutime, cstime;
    int num_scanned;
    char state;
    if ((num_scanned = sscanf(buffer, format, name, &state, &utime, &stime, &cutime, &cstime)) < 6) {
        fprintf(stderr, "Error while reading proc stat file: %s (%d) (%d) (%s)\n", stat_file_name, num_scanned,
                *name_length, name);
        rewind(fp);
        char c;
        while ((c = (char) fgetc(fp)) != EOF) {
            fprintf(stderr, "%c", c);
        }
        fprintf(stderr, "\n\n");
        return -1;
    } else {
        *time = utime + stime + cutime + cstime;
        // printf("# %s (%ld): %lu\n", name, pid, *time);
        return 0;
    }
}

void
print_process_records(int my_rank, int procs, const process_record *process_records, int number_of_process_records) {
    int root = OUTPUT_ROOT_PROC;

    // Init MPI Type
    MPI_Datatype mpi_process_record;
    int array_of_block_lengths[] = {1, 64, 1, 1, 256, 1};

    MPI_Aint array_of_displacements[] = {
            offsetof(process_record, rank),
            offsetof(process_record, hostname),
            offsetof(process_record, nrep),
            offsetof(process_record, pid),
            offsetof(process_record, process_name),
            offsetof(process_record, time)
    };
    MPI_Datatype array_of_types[] = {MPI_INT, MPI_CHAR, MPI_LONG, MPI_LONG, MPI_CHAR, MPI_UNSIGNED_LONG};

    MPI_Type_create_struct(6, array_of_block_lengths, array_of_displacements, array_of_types, &mpi_process_record);
    MPI_Type_commit(&mpi_process_record);

    // Gather sizes
    int *numbers_of_process_records;
    int *displs;
    if (my_rank == root) {
        numbers_of_process_records = malloc(sizeof(int) * procs);
        displs = malloc(sizeof(int) * procs);
    }
    MPI_Gather(&number_of_process_records, 1, MPI_INT, numbers_of_process_records, 1, MPI_INT, root, MPI_COMM_WORLD);

    process_record *process_receive_buffer;
    int sum_process_records = 0;
    if (my_rank == root) {
        for (int i = 0; i < procs; i++) {
            displs[i] = sum_process_records;
            sum_process_records += numbers_of_process_records[i];
        }
        if (my_rank == root) {
            process_receive_buffer = malloc(sum_process_records * sizeof(process_record));
        }
    }
    MPI_Gatherv(process_records, number_of_process_records, mpi_process_record, process_receive_buffer,
                numbers_of_process_records, displs, mpi_process_record, root, MPI_COMM_WORLD);
    if (my_rank == root) {
        printf("# Number of process_records: %d\n", number_of_process_records * procs);
        printf("nrep,rank,global_pid,time\n");
        for (int j = 0; j < sum_process_records; j++) {
            if (process_receive_buffer[j].time != 0 &&
                strncmp(process_receive_buffer[j].process_name, "mpibenchmark", 12) != 0) {
                printf("%ld,%d,%s:%s(%ld),%lu\n",
                       process_receive_buffer[j].nrep,
                       process_receive_buffer[j].rank,
                       process_receive_buffer[j].hostname,
                       process_receive_buffer[j].process_name,
                       process_receive_buffer[j].pid,
                       process_receive_buffer[j].time);
                fflush(stdout);
            }
        }
    }
}

