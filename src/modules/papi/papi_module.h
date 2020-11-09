//
// Created by niklas on 09.11.20.
// either includes the papi header or NOP macros to avoid compile errors
//

#ifndef REPROMPI_PAPI_MODULE_H
#define REPROMPI_PAPI_MODULE_H

#ifdef PAPI

#include <papi.h>
void handle_error(int retval);

#else

#define PAPI_OK 0
#define PAPI_strerror(retVal) ""
#define PAPI_hl_region_begin(region) PAPI_OK
#define PAPI_hl_region_end(region) PAPI_OK
#define PAPI_hl_stop() PAPI_OK
#define handle_error(retval)

#endif

#endif //REPROMPI_PAPI_MODULE_H
