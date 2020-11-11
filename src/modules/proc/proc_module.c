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

void init_pids() {
    num_pids = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (isNumeric(dir->d_name)) {
                num_pids++;
                pids = realloc(pids, sizeof(long) * num_pids);
                char *ignore = NULL;
                (pids)[num_pids - 1] = strtol(dir->d_name, &ignore, 10);
                //printf("# pid_%d: %ld\n", pid_cnt-1, (*pids)[pid_cnt - 1]);
            }
        }
        closedir(d);
    }
}

void init_proc_module(long nrep) {
    init_pids();
    process_prev_times = malloc(sizeof(unsigned long) * num_pids);
    number_of_process_records = (int) nrep * num_pids;
    process_records = malloc(number_of_process_records * sizeof(process_record));
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
        fprintf(stderr, "Couldn't open file %s, Error %d: %s\n", stat_file_name, errno, strerror(errno));
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

void save_process_prev_times() {
    for (int j = 0; j < num_pids; j++) {
        unsigned long time;
        long pid = pids[j];
        char name[256];
        int name_length;
        get_info_from_pid(pid, &name_length, name, &time);
        process_prev_times[j] = time;
    }
}

void save_process_records(long repetition_id, int my_rank) {
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

        long index = num_pids * repetition_id + i;
        process_records[index].time = time;
        process_records[index].pid = pid;
        process_records[index].rank = my_rank;
        process_records[index].nrep = repetition_id;
        strncpy(process_records[index].process_name, name, name_length + 1);
        gethostname(process_records[index].hostname, 64);
        // printf("## #%ld Rank_%d: %s (%ld): %lu\n", repetition_id, my_rank, process_records[index].process_name, process_records[index].pid, process_records[index].time);
        fflush(stdout);
    }
}

void print_process_records(int my_rank, int root, int procs) {

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
    if (my_rank == root) {
        numbers_of_process_records = malloc(sizeof(int) * procs);
        displs = malloc(sizeof(int) * procs);
    }
    MPI_Gather(&number_of_process_records, 1, MPI_INT, numbers_of_process_records, 1, MPI_INT, root, MPI_COMM_WORLD);

    process_record *process_receive_buffer = NULL;
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

#endif //PROC_MODULE
