//
// Created by niklas on 11.11.20.
//


#include "proc_module.h"

#ifdef PROC_MODULE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <mpi.h>

typedef struct process_record {
    int rank;
    char hostname[64];
    long nrep;
    long pid;
    char process_name[256];
    unsigned long time;
} process_record;

long *pids = 0;
int num_pids = 0;
int number_of_process_records = 0;
unsigned long *process_prev_times = NULL;
process_record *process_records = NULL;
int global_rank;
int shm_rank;
int SHM_ROOT = 0;
MPI_Comm one_of_each_node_comm;
int node_rank;
int NODE_ROOT = 0;
int number_of_nodes;

long number_of_repetitions;

long last_repetition = -1;

bool isNumeric(char *string) {
    // Get value with failure detection.

    char *next;
    strtol(string, &next, 10);

    // Check for empty string and characters left after conversion.

    if ((next == string) || (*next != '\0')) {
        return false;
    } else {
        return true;
    }
}

bool is_tracked(long pid) {
    for (int i = 0; i < num_pids; i++) {
        if (pids[i] == pid) {
            return true;
        }
    }
    return false;
}

int get_info_from_pid(long pid, int *name_length, char *name, unsigned long *time) {
    FILE *fp = NULL;
    char stat_file_name[20];
    snprintf(stat_file_name, 20, "/proc/%ld/stat", pid);
    int buffer_size = 1024;
    char buffer[buffer_size];
    // strncpy(buffer,
    //         "1 (systemd) S 0 1 1 0 -1 4194560 109962 2745609 95 1915 1334 6360 6260 5589 20 0 1 0 13 172765184 3135 18446744073709551615 1 1 0 0 0 0 671173123 4096 1260 0 0 0 17 0 0 0 215 0 0 0 0 0 0 0 0 0 0",
    //         buffer_size);
    fp = fopen(stat_file_name, "r");
    if (fp == NULL) {
        // fprintf(stderr, "Couldn't open file %s, Error %d: %s\n", stat_file_name, errno, strerror(errno));
        return errno;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-result"
    fgets(buffer, buffer_size, fp);
#pragma clang diagnostic pop
    fclose(fp);
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

void save_process_record(long repetition_id, int pid_index, bool set_time_zero) {
    long pid = pids[pid_index];
    //printf("## Saving pid: %ld\n", pid);
    //fflush(stdout);
    unsigned long time;
    int name_length;
    char name[256];
    if (get_info_from_pid(pid, &name_length, name, &time) != 0) {
        time = 0;
        strncpy(name, "", 1);
        name_length = 0;
    }
    //printf("## Name: %s\n", name);
    //fflush(stdout)
    if (time < process_prev_times[pid_index]) {
        fprintf(stderr, "Warning @ %d: %s (%ld): time after: %lu < time previous %lu", global_rank, name, pid, time,
                process_prev_times[pid_index]);
        time = 0L;
    } else {
        time -= process_prev_times[pid_index];
    }

    if (set_time_zero) {
        time = 0;
    }
    time = time * (1000000 / sysconf(_SC_CLK_TCK));
    long index = number_of_repetitions * pid_index + repetition_id;
    process_records[index].time = time;
    process_records[index].pid = pid;
    process_records[index].rank = global_rank;
    process_records[index].nrep = repetition_id;
    strncpy(process_records[index].process_name, name, name_length + 1);
    gethostname(process_records[index].hostname, 64);
    // printf("## #%ld Rank_%d: %s (%ld): %lu\n", repetition_id, my_rank, process_records[index].process_name, process_records[index].pid, process_records[index].time);
    // fflush(stdout);
}

void add_untracked_pids() {
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (isNumeric(dir->d_name)) {
                char *ignore = NULL;
                long pid = strtol(dir->d_name, &ignore, 10);
                if (!is_tracked(pid)) {
                    num_pids++;
                    pids = realloc(pids, num_pids * sizeof(long));
                    process_prev_times = realloc(process_prev_times, num_pids * sizeof(unsigned long));
                    number_of_process_records = (int) number_of_repetitions * num_pids;
                    process_records = realloc(process_records, number_of_process_records * sizeof(process_record));
                    pids[num_pids - 1] = pid;
                    process_prev_times[num_pids - 1] = 0;
                    for (int i = 0; i <= last_repetition; i++) {
                        save_process_record(i, num_pids - 1, true);
                    }
                }
                //printf("# pid_%d: %ld\n", pid_cnt-1, (*pids)[pid_cnt - 1]);
            }
        }
        closedir(d);
    }
}

void init_proc_module(long nrep) {
    MPI_Comm shm_comm;
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0,
                        MPI_INFO_NULL, &shm_comm);
    MPI_Comm_rank(shm_comm, &shm_rank);
    MPI_Comm_split(MPI_COMM_WORLD, shm_rank, global_rank, &one_of_each_node_comm);
    MPI_Comm_free(&shm_comm);
    MPI_Comm_size(one_of_each_node_comm, &number_of_nodes);
    MPI_Comm_rank(one_of_each_node_comm, &node_rank);
    if (shm_rank == SHM_ROOT) {
        num_pids = 0;
        last_repetition = -1;
        number_of_repetitions = nrep;
        add_untracked_pids();
    }
}


void save_process_prev_times() {
    if (shm_rank == SHM_ROOT) {
        for (int j = 0; j < num_pids; j++) {
            unsigned long time;
            long pid = pids[j];
            char name[256];
            int name_length;
            if (get_info_from_pid(pid, &name_length, name, &time) != 0) {
                time = 0;
            }
            process_prev_times[j] = time;
        }
    }
}


void save_process_records(long repetition_id) {
    if (shm_rank == SHM_ROOT) {
        for (int i = 0; i < num_pids; i++) {
            save_process_record(repetition_id, i, false);
        }
        last_repetition = repetition_id;
    }
}

void print_process_records() {
    if (shm_rank == SHM_ROOT) {
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
        int *numbers_of_process_records = NULL;
        int *displs = NULL;
        if (node_rank == NODE_ROOT) {
            numbers_of_process_records = malloc(sizeof(int) * number_of_nodes);
            displs = malloc(sizeof(int) * number_of_nodes);
        }
        MPI_Gather(&number_of_process_records, 1, MPI_INT, numbers_of_process_records, 1, MPI_INT, NODE_ROOT,
                   one_of_each_node_comm);

        process_record *process_receive_buffer = NULL;
        int sum_process_records = 0;
        if (node_rank == NODE_ROOT) {
            for (int i = 0; i < number_of_nodes; i++) {
                displs[i] = sum_process_records;
                sum_process_records += numbers_of_process_records[i];
            }
            if (node_rank == NODE_ROOT) {
                process_receive_buffer = malloc(sum_process_records * sizeof(process_record));
            }
        }
        MPI_Gatherv(process_records, number_of_process_records, mpi_process_record, process_receive_buffer,
                    numbers_of_process_records, displs, mpi_process_record, NODE_ROOT, one_of_each_node_comm);
        if (node_rank == NODE_ROOT) {
            printf("# Number of process_records: %d\n", number_of_process_records * number_of_nodes);
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
}

#endif //PROC_MODULE
