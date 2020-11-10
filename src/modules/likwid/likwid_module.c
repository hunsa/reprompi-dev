//
// Created by niklas on 09.11.20.
//

#include <stdlib.h>
#include "likwid_module.h"
#include "modules/utils.h"

#ifdef LIKWID_PERFMON

void initialize_likwid_regions(long nrep) {
    for (long i = 0; i < nrep; i++) {
        char *region_name = get_region_name(i, nrep);
        LIKWID_MARKER_REGISTER(region_name);
        free(region_name);
    }
}

#endif