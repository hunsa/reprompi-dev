import sys
import os
import re

PARSED_OPTS_VAR = "reprompib_opts"
JOB_VAR_NAME = "reprompib_job"
NREP_INDEX_VAR_NAME = "reprompib_nrep_index"
SYNCF_VAR_NAME = "clock_sync"
PROCSYNC_VAR_NAME = "proc_sync"
SYNC_PARAMS_VAR_NAME = "sync_params"
NREP_VAR_NAME = "cur_nrep"

def generate_init_bench(indent):
    code = [ "reprompib_initialize_benchmark(argc, argv, &%s, &%s, &%s);" % (PARSED_OPTS_VAR, SYNCF_VAR_NAME, PROCSYNC_VAR_NAME)
             ]
    return format_code(code, indent)


def generate_init_sync(indent):
    code = [ 
            "sync_params.nrep  = reprompib_opts.n_rep;",
            "sync_params.count = 0;",
            "%s.init_sync(&%s);" % (PROCSYNC_VAR_NAME, SYNC_PARAMS_VAR_NAME),
            "%s.init_sync();" % (SYNCF_VAR_NAME),
            "",
            "%s.sync_clocks();" % (SYNCF_VAR_NAME), 
            "%s.init_sync_round(); " % (PROCSYNC_VAR_NAME)            
            ]
    return format_code(code, indent)



def generate_start_sync(indent):
    code = [ "%s.start_sync();" % (PROCSYNC_VAR_NAME) ]
    return format_code(code, indent)


def generate_stop_sync(indent):
    code = [ "%s.stop_sync();" % (PROCSYNC_VAR_NAME) ]
    return format_code(code, indent)


def generate_measure_timestamp(ts, indent):
    code = [ "%s[%s] = REPROMPI_get_time();" % (ts, NREP_INDEX_VAR_NAME) ]
    return format_code(code, indent)


def generate_print_output(output_config, indent):
    code = [
            "\treprompib_print_bench_output(&%s, &%s, &%s, &%s); " % \
            (JOB_VAR_NAME, SYNCF_VAR_NAME, PROCSYNC_VAR_NAME, PARSED_OPTS_VAR)
            ]

    return format_code("{", indent) + generate_init_job(output_config, indent) + \
            format_code(code, indent) + generate_cleanup_job(indent) + format_code("}", indent) +"\n"


#def generate_cleanup_sync(indent):

#     code = [         
#         "%s.finalize_sync();" % (SYNCF_VAR_NAME),
#         "%s.finalize_sync();" % (PROCSYNC_VAR_NAME),
#         "%s.cleanup_module();" % (SYNCF_VAR_NAME),
#         "%s.cleanup_module();" % (PROCSYNC_VAR_NAME)        
#          ]
     
#    return format_code(code, indent)


def generate_cleanup_variables(ts_array, strings_array, indent):
    cleanup_ts_arrays_code = generate_cleanup_arrays(ts_array, indent)
    cleanup_strings_array_code = generate_cleanup_arrays(strings_array, indent)

    return cleanup_ts_arrays_code + cleanup_strings_array_code + "\n"


def generate_cleanup_bench(indent):
#    cleanup_sync_code = generate_cleanup_sync(indent)

    code = [ "reprompib_cleanup_benchmark(&%s, &%s, &%s);" % (
        PARSED_OPTS_VAR,
        SYNCF_VAR_NAME,
        PROCSYNC_VAR_NAME
        ) ]
    return format_code(code, indent)
    
 #   return cleanup_sync_code + format_code(code, indent)



# def generate_declare_variables_main_file(indent):
#
#     return format_code(code, indent)



def generate_declare_variables(main_file, ts_arrays, strings_array, indent):

    code = [         
        "@@reprompib_sync_module_t %s;" % (SYNCF_VAR_NAME),
        "@@reprompib_proc_sync_module_t %s;" % (PROCSYNC_VAR_NAME),
        "@@reprompib_sync_params_t %s;" % (SYNC_PARAMS_VAR_NAME),
        "@@reprompib_options_t %s;" % (PARSED_OPTS_VAR),
    ]
    
    code += [
        "long %s;" % (NREP_INDEX_VAR_NAME),
        "int %s;" % (NREP_VAR_NAME),
        "int is_invalid;",
        "reprompib_job_t %s;" % (JOB_VAR_NAME),
    ]

    if main_file:
        code = list(map(lambda x: x.replace("@@",""), code))
    else:
        code = list(map(lambda x: x.replace("@@","extern "), code))

    ts_array_unique = list(set(ts_arrays))
    additional_vars1 = list(map((lambda t: "double* %s = NULL;" % (t) ), ts_array_unique))
    strings_array_unique = list(set(strings_array))
    additional_vars2 = list(map((lambda s: "char* %s = NULL;" % (s) ), strings_array_unique))

    code.extend(additional_vars1)
    code.extend(additional_vars2)

    return format_code(code, indent)


def generate_add_includes(indent):
    code = [ """#include <string.h>
#include "reprompi_bench/sync/process_sync/process_synchronization.h"
#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/benchmark_lib/reproMPIbenchmark.h"
#include "reprompi_bench/sync/time_measurement.h"
"""
            ]
    return format_code(code, indent)


def generate_init_timestamp_array(name, indent):
    code = [ "%s = (double*) calloc(%s.n_rep, sizeof(double));" % (name, PARSED_OPTS_VAR) ]
    return format_code(code, indent)


def generate_cleanup_arrays(array, indent):
    array_unique = list(set(array))
    code_free = list(map((lambda t: "free(%s);" % (t) ), array_unique))
    code_set_null = list(map((lambda t: "%s = NULL;" % (t) ), array_unique))
    return format_code(code_free, indent) + format_code(code_set_null, indent)


def generate_init_job(output_config, indent):
    svars = output_config["string_list"]
    ivars = output_config["int_list"]

    #print "\n\n$$$$$$$$$$$$$$$$$$$$$$$$"
    print("svars:", svars)
    print("ivars:", ivars)
    #print "#########################"

    job_config = [
            #"\treprompib_job_t %s;" % (JOB_VAR_NAME),
            "\treprompib_initialize_job(%s, %s, %s, " % (NREP_VAR_NAME, output_config["start_time"], output_config["end_time"]),
             "\t        \"%s\", \"%s\", \"%s\", &%s);"  % (output_config["op"], output_config["name"], output_config["type"], JOB_VAR_NAME)
            ]
    job_config.extend(list(map(lambda tup: "\treprompib_add_svar_to_job(\"%s\", %s, &%s);" % (tup[0], tup[1], JOB_VAR_NAME), svars.items())))
    job_config.extend(list(map(lambda tup: "\treprompib_add_ivar_to_job(\"%s\", %s, &%s);" % (tup[0], tup[1], JOB_VAR_NAME), ivars.items())))

    return format_code(job_config, indent)

def generate_cleanup_job(indent):
    code = [ "\treprompib_cleanup_job(&%s);" % (JOB_VAR_NAME)
            ]
    return format_code(code, indent)


def generate_start_measurement_loop(indent):
    spaces = ' ' * indent
#    init_job_code = generate_init_job(test_var, msize_var, indent)
    init_sync_code = generate_init_sync(indent)
    
#    code = ["long %s;" % (NREP_INDEX_VAR_NAME)]
#     
#     code += [ "for (%s = 0; %s < %s.n_rep; %s++) { " % (NREP_INDEX_VAR_NAME,
#                                                        NREP_INDEX_VAR_NAME, PARSED_OPTS_VAR,
#                                                        NREP_INDEX_VAR_NAME)
#             ]
    
    code = \
"""
        %s = %s.n_rep;
        %s = 0;
        while(%s < %s) {
          proc_sync.start_sync(MPI_COMM_WORLD);
""" % ( NREP_VAR_NAME,
        PARSED_OPTS_VAR,
        NREP_INDEX_VAR_NAME,
        NREP_INDEX_VAR_NAME,
        NREP_VAR_NAME)

    #return init_sync_code + format_code(code, indent)
    return init_sync_code + code


def generate_stop_measurement_loop(indent):
    
    code = \
"""
          is_invalid = proc_sync.stop_sync(MPI_COMM_WORLD);
          if (is_invalid == REPROMPI_INVALID_MEASUREMENT) {
            // redo the measurement
            // we are still in the time frame
          } else if( is_invalid == REPROMPI_OUT_OF_TIME_VALID ) {
            %s = %s+1;
            break;
          } else if( is_invalid == REPROMPI_OUT_OF_TIME_INVALID ) {
            %s = MY_MAX(0, %s-1);
            break;
          } else {
            %s++;
          }
          if (%s == %s) {
            break;
          }
    }
""" % ( NREP_VAR_NAME, NREP_INDEX_VAR_NAME, NREP_VAR_NAME, NREP_INDEX_VAR_NAME, NREP_INDEX_VAR_NAME, NREP_VAR_NAME, NREP_INDEX_VAR_NAME )
    
    #code = "}"
    #return format_code(code, indent)
    return code


def generate_add_to_dictionary(dict, indent):
    spaces = ' ' * indent
    code = map((lambda t: "reprompib_add_parameter_to_bench(\"%s\", %s);" % (t, dict[t]) ), dict.keys())
    return format_code(code, indent)


def generate_set_variable(dict, indent):
    code = map((lambda t: "%s = strdup(%s);" % (t, dict[t]) ), dict.keys())
    return format_code(code, indent)


def format_code(code_lines, indent):
    spaces = ' ' * indent
    code = map((lambda t: "%s%s\n" % (spaces, t) ), code_lines)
    return "".join(code)
