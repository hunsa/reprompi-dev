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
#include <stdlib.h>
#include <getopt.h>

#include "sync_info.h"


const int REPROMPI_SYNC_N_FITPOINTS_DEFAULT = 20;
const int REPROMPI_SYNC_N_EXCHANGES_DEFAULT = 10;


const struct option reprompi_sync_long_options[] = {
        { "fitpoints", required_argument, 0, REPROMPI_ARGS_CLOCKSYNC_NFITPOINTS },
        { "exchanges", required_argument, 0, REPROMPI_ARGS_CLOCKSYNC_NEXCHANGES },
        { 0, 0, 0, 0 }
};
const char reprompi_sync_opts_str[] = "";




void reprompi_init_sync_parameters(reprompib_sync_options_t* opts_p) {
  opts_p->n_fitpoints = REPROMPI_SYNC_N_FITPOINTS_DEFAULT;
  opts_p->n_exchanges = REPROMPI_SYNC_N_EXCHANGES_DEFAULT;

}
