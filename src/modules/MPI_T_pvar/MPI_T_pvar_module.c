//
// Created by niklas on 10.11.20.
//

#ifdef MPI_T_PVARS

#include "MPI_T_pvar_module.h"
#include "mpi.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

typedef struct pvar_record {
    int rank;
    long nrep;
    char pvar_name[256];
    long long value;
} pvar_record;

pvar_record *pvar_records = NULL;
char **pvar_names;
int number_of_pvars;
long number_of_pvar_records;
long long int *pvar_prev_values;
MPI_T_pvar_session session;
MPI_T_pvar_handle *pvar_handles;

void init_handles_for_MPI_T_pvars() {
    //printf("Init handles\n");
    //fflush(stdout);
    int i, num, name_len, desc_len, verbosity, bind, var_class, readonly, continuous, atomic, count, index;
    MPI_Datatype datatype;
    MPI_T_enum enumtype;
    pvar_handles = malloc(number_of_pvars * sizeof(MPI_T_pvar_handle));
    char name[256], description[256];

    for (int v = 0; v < number_of_pvars; v++) {
        index = -1;
        MPI_T_pvar_get_num(&num);
        //printf("Creating handle for pvar nr. %d\n", v);
        //fflush(stdout);
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


void init_MPI_T_pvars(long nrep) {
    int provided;
    //printf("Init Thread\n");
    //fflush(stdout);
    MPI_T_init_thread(MPI_THREAD_SINGLE, &provided);
    get_all_MPI_T_pvars(&pvar_names, &number_of_pvars);
    // Uncomment this if you want to only measure the specified pvars and not all available
    /*char *pvar_names[] = {"runtime_spc_OMPI_SPC_BYTES_RECEIVED_USER",
                          "runtime_spc_OMPI_SPC_BYTES_RECEIVED_MPI",
                          "runtime_spc_OMPI_SPC_BYTES_SENT_USER",
                          "runtime_spc_OMPI_SPC_BYTES_SENT_MPI"};
    int number_of_pvars = NELEMS(pvar_names);*/
    pvar_prev_values = malloc(number_of_pvars * sizeof(long long int));

    MPI_T_pvar_session_create(&session);
    init_handles_for_MPI_T_pvars();

    for (int j = 0; j < number_of_pvars; j++) {
        // printf("# Index for pvar %s: %s\n", pvar_names[j], pvar_handles[j]->pvar->name);
        MPI_T_pvar_start(session, pvar_handles[j]);
    }
    number_of_pvar_records = nrep * number_of_pvars;
    pvar_records = malloc(number_of_pvar_records * sizeof(pvar_record));
    //printf("Finished initializing\n");
    //fflush(stdout);
}

void start_MPI_T_pvars() {
    for (int j = 0; j < number_of_pvars; j++) {
        long long int value;
        MPI_T_pvar_reset(session, pvar_handles[j]);
        MPI_T_pvar_read(session, pvar_handles[j], &value);
        pvar_prev_values[j] = value;
    }
}

void save_MPI_T_pvars(long repetition_id, int my_rank) {
    for (int j = 0; j < number_of_pvars; j++) {
        pvar_record pvar_record;
        pvar_record.nrep = repetition_id;
        pvar_record.rank = my_rank;
        strcpy(pvar_record.pvar_name, pvar_names[j]);
        long long int value;
        MPI_T_pvar_read(session, pvar_handles[j], &value);
        pvar_record.value = value - pvar_prev_values[j];
        pvar_records[repetition_id * number_of_pvars + j] = pvar_record;
    }

}

void print_pvars(int my_rank, int root, int procs) {
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

    pvar_record *pvars_receive_buffer = NULL;
    if (my_rank == root) {
        pvars_receive_buffer = malloc(procs * number_of_pvar_records * sizeof(pvar_record));
    }
    MPI_Gather(pvar_records, (int) number_of_pvar_records, mpi_pvar_record,
               pvars_receive_buffer, (int) number_of_pvar_records, mpi_pvar_record,
               root, MPI_COMM_WORLD);
    if (my_rank == root) {
        printf("# Number of pvar_records: %ld\n", number_of_pvar_records * procs);
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

void clean_up_MPI_T_pvars() {
    for (int j = 0; j < number_of_pvars; j++) {
        MPI_T_pvar_stop(session, pvar_handles[j]);
    }
    MPI_T_pvar_session_free(&session);
    MPI_T_finalize();
}

#endif