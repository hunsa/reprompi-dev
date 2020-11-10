//
// Created by niklas on 09.11.20.
//

#include "utils.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"

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