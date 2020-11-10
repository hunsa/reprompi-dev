//
// Created by niklas on 09.11.20.
//

#ifndef REPROMPI_LIKWID_MODULE_H
#define REPROMPI_LIKWID_MODULE_H

#ifdef LIKWID_PERFMON

#include <likwid.h>
void initialize_likwid_regions(long nrep);

#else
#define LIKWID_MARKER_INIT
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_SWITCH
#define LIKWID_MARKER_REGISTER(regionTag)
#define LIKWID_MARKER_START(regionTag)
#define LIKWID_MARKER_STOP(regionTag)
#define LIKWID_MARKER_CLOSE
#define LIKWID_MARKER_GET(regionTag, nevents, events, time, count)
#define initialize_likwid_regions(nrep)
#endif

#endif //REPROMPI_LIKWID_MODULE_H
