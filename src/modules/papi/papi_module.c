//
// Created by niklas on 09.11.20.
//

#include "papi_module.h"
#ifdef PAPI

void handle_error(int retval) {
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    exit(1);
}

#endif

