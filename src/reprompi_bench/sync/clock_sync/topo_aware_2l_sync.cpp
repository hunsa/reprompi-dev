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

#include <cstdio>
#include <string>
#include <cstdlib>
#include <vector>
#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/RdtscpClock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_common.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_lib.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/TwoLevelClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/SKaMPIClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/JKClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockPropagationSync.h"
#include "reprompi_bench/sync/common/sync_module_helpers.h"
#include "reprompi_bench/misc.h"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;

static ClockOffsetAlg *instantiate_clock_offset_alg(std::vector<std::string> &tokens);
static ClockSync* instantiate_clock_sync(const char *param_name);
static std::vector<std::string> str_split(const char *str, char c);

//////////////////////////////////////////////////////////////////////////////

static std::vector<std::string> str_split(const char *str, char c = ' ') {
  std::vector<std::string> result;

  do {
    const char *begin = str;

    while (*str != c && *str)
      str++;

    result.push_back(std::string(begin, str));
  } while (0 != *str++);

  return result;
}

static void topo_synchronize_clocks(void) {
  global_clock = clock_sync->synchronize_all_clocks(MPI_COMM_WORLD, *(local_clock));
}

static double topo_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}


static void topo_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}



static void topo_print_sync_parameters(FILE* f)
{
  fprintf (f, "#@clocksync=TwoLevelSync\n");
//  fprintf(f, "#@fitpoints=%d\n", parameters.n_fitpoints);
//  fprintf(f, "#@exchanges=%d\n", parameters.n_exchanges);
}

//static void topo2_print_sync_parameters(FILE* f)
//{
//  fprintf (f, "#@clocksync=Topo2\n");
////  fprintf(f, "#@fitpoints=%d\n", parameters.n_fitpoints);
////  fprintf(f, "#@exchanges=%d\n", parameters.n_exchanges);
//}

static ClockOffsetAlg *instantiate_clock_offset_alg(std::vector<std::string> &tokens) {
  ClockOffsetAlg *offset_alg = NULL;

  if( tokens[0] == "skampi_offset" ) {
    // skampi_offset,min_nb_ping_pongs,nb_ping_pongs
    if( tokens.size() != 3 ) {
      ZF_LOGE("number of parameters to ClockOffsetAlg wrong (!=3)");
    } else {
      int min_nb_ping_pongs = atoi(tokens[1].c_str());
      int nb_ping_pongs     = atoi(tokens[2].c_str());
      ZF_LOGV("skampi offset with %d,%d ping-pongs", min_nb_ping_pongs, nb_ping_pongs);
      offset_alg = new SKaMPIClockOffsetAlg(min_nb_ping_pongs, nb_ping_pongs);
    }
  } else if( tokens[0] == "pingpong_offset" ) {
    // pingpong_offset,nexchanges_rtt,nexchanges
    if( tokens.size() != 3 ) {
      ZF_LOGE("number of parameters to ClockOffsetAlg wrong (!=3)");
    } else {
      int nexchanges_rtt = atoi(tokens[1].c_str());
      int nexchanges     = atoi(tokens[2].c_str());
      ZF_LOGV("ping-pong offset with %d,%d exchanges", nexchanges_rtt, nexchanges);
      offset_alg = new PingpongClockOffsetAlg(nexchanges_rtt, nexchanges);
    }
  } else {
    ZF_LOGE("unknown offset algorithm '%s'", tokens[0].c_str());
  }

  return offset_alg;
}

static ClockSync* instantiate_clock_sync(const char *param_name) {
  ClockSync* ret_sync = NULL;
  char *alg_str;
  reprompib_dictionary_t *dict = get_global_param_store();

  if( reprompib_dict_has_key(dict, param_name) == 1 ) {
    reprompib_get_value_from_dict(dict, param_name, &alg_str);

   std::vector<std::string> tokens = str_split(alg_str, '@');
   if( tokens.size() <= 0 ) {
     ZF_LOGE("value of %s incompatible", param_name);
   } else {
     std::string sync_alg = tokens[0];

     tokens.erase(tokens.begin());

     if( sync_alg == "hca2" || sync_alg == "hca3" || sync_alg == "jk" ) {

       if( tokens.size() >= 1 ) {
         // get: number of fitpoints
         int n_fitpoints = atoi(tokens[0].c_str());
         tokens.erase(tokens.begin());

         ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
         if( offset_alg == NULL ) {
           ZF_LOGE("cannot instantiate clock offset algorithm");
         } else {
           if( sync_alg == "hca2" ) {
             ZF_LOGV("hca2 clock sync with %d fitpoints", n_fitpoints);
             ret_sync = new HCA2ClockSync(offset_alg, n_fitpoints);
           } else if( sync_alg == "hca3" ) {
             ZF_LOGV("hca3 clock sync with %d fitpoints", n_fitpoints);
             ret_sync = new HCA3ClockSync(offset_alg, n_fitpoints);
           } else if( sync_alg == "jk" ) {
             ZF_LOGV("jk clock sync with %d fitpoints", n_fitpoints);
             ret_sync = new JKClockSync(offset_alg, n_fitpoints);
           }
         }

       } else {
         ZF_LOGE("format error sync alg '%s'", sync_alg.c_str());
       }

     } else if( sync_alg == "skampi" ) {
       if( tokens.size() >= 1 ) {
         ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
         ret_sync = new SKaMPIClockSync(offset_alg);
       } else {
         ZF_LOGE("format error sync alg '%s'", sync_alg.c_str());
       }
     } else if( sync_alg == "prop" ) {
       ret_sync = new ClockPropagationSync();
     } else {
       ZF_LOGE("unknown clock sync alg '%s'", sync_alg.c_str());
     }
   }

  } else {
    ZF_LOGE("parameter '%s' not found, using default", param_name);
  }

  return ret_sync;
}

static void topo_init_module(int argc, char** argv) {
  int use_default = 0;
  ClockSync *alg1;
  ClockSync *alg2;

  alg1 = instantiate_clock_sync("topoalg1");
  if( alg1 != NULL ) {
    alg2 = instantiate_clock_sync("topoalg2");
    if( alg2 != NULL ) {
      // now instantiate new two level clock sync
      clock_sync = new TwoLevelClockSync(alg1, alg2);
    } else {
      use_default = 1;
    }
  } else {
    use_default = 1;
  }

  global_clock = NULL;
  local_clock  = initialize_local_clock();

  if( use_default == 1 ) {
    ZF_LOGW("!!! using default topo1 clock sync options");

    clock_sync = new TwoLevelClockSync(
      new HCA3ClockSync(new SKaMPIClockOffsetAlg(10,100), 500),
      new ClockPropagationSync());
  }

}


extern "C"
void register_topo_aware_sync2_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("Topo2").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_TOPO2;
  sync_mod->init_module = topo_init_module;
  sync_mod->cleanup_module = topo_cleanup_module;
  sync_mod->sync_clocks = topo_synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = topo_normalized_time;
  sync_mod->print_sync_info = topo_print_sync_parameters;
}


