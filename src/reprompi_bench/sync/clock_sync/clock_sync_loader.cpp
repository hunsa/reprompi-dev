
#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "clock_sync_loader.hpp"

#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HierarchicalClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/SKaMPIClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/JKClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockPropagationSync.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_common.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_lib.h"
#include "reprompi_bench/sync/common/sync_module_helpers.h"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static std::vector<std::string> str_split(const char *str, char c);
static ClockOffsetAlg* instantiate_clock_offset_alg(std::vector<std::string> &tokens);


static std::vector<std::string> str_split(const char *str, char c = ' ') {
  std::vector<std::string> result;

  do {
    const char *begin = str;

    while (*str != c && *str) {
      str++;
    }

    result.push_back(std::string(begin, str));
  } while (0 != *str++);

  return result;
}


static ClockOffsetAlg* instantiate_clock_offset_alg(std::vector<std::string> &tokens) {
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

ClockSync* ClockSyncLoader::instantiate_clock_sync(const char *param_name) {
  ClockSync* ret_sync = NULL;
  char *alg_str;
  reprompib_dictionary_t *dict = get_global_param_store();
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //ZF_LOGV("instantiate clock for %s", param_name);

  if (reprompib_dict_has_key(dict, param_name) == 1) {
      reprompib_get_value_from_dict(dict, param_name, &alg_str);

      std::vector<std::string> tokens = str_split(alg_str, '@');
      free(alg_str);
      if (tokens.size() <= 0) {
          ZF_LOGE("value of %s incompatible", param_name);
      } else {
          std::string sync_alg = tokens[0];

          tokens.erase(tokens.begin());

          if (sync_alg == "hca2" || sync_alg == "hca3") {

              // hca2@recompute_offsets@fitpoints@offsetalg_format
              // hca3@recompute_offsets@fitpoints@offsetalg_format
              if (tokens.size() >= 2) {

                  bool recompute_offset = (atoi(tokens[0].c_str()) == 1);
                  tokens.erase(tokens.begin());

                  // get: number of fitpoints
                  int n_fitpoints = atoi(tokens[0].c_str());
                  tokens.erase(tokens.begin());

                  ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
                  if (offset_alg == NULL) {
                      ZF_LOGE("cannot instantiate clock offset algorithm");
                  } else {
                      if (sync_alg == "hca2") {
                          ZF_LOGV("hca2 clock sync with %d fitpoints, recompute %d", n_fitpoints, recompute_offset);
                          ret_sync = new HCA2ClockSync(offset_alg, n_fitpoints, recompute_offset);
                      } else if (sync_alg == "hca3") {
                          ZF_LOGV("hca3 clock sync with %d fitpoints, recompute %d", n_fitpoints, recompute_offset);
                          ret_sync = new HCA3ClockSync(offset_alg, n_fitpoints, recompute_offset);
                      }
                  }
              } else {
                  ZF_LOGE("format error sync alg '%s'", sync_alg.c_str());
              }

          } else if (sync_alg == "hca" || sync_alg == "jk") {

              // hca@fitpoints@offsetalg_format
              //  jk@fitpoints@offsetalg_format
              if (tokens.size() >= 1) {
                  // get: number of fitpoints
                  int n_fitpoints = atoi(tokens[0].c_str());
                  tokens.erase(tokens.begin());

                  ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
                  if (offset_alg == NULL) {
                      ZF_LOGE("cannot instantiate clock offset algorithm");
                  } else {
                      if (sync_alg == "hca") {
                          ZF_LOGV("hca clock sync with %d fitpoints", n_fitpoints);
                          ret_sync = new HCAClockSync(offset_alg, n_fitpoints);
                      } else if (sync_alg == "jk") {
                          ZF_LOGV("jk clock sync with %d fitpoints", n_fitpoints);
                          ret_sync = new JKClockSync(offset_alg, n_fitpoints);
                      }
                  }
              } else {
                  ZF_LOGE("format error sync alg '%s'", sync_alg.c_str());
              }

          } else if (sync_alg == "skampi") {
              if (tokens.size() >= 1) {
                  ClockOffsetAlg *offset_alg = instantiate_clock_offset_alg(tokens);
                  if (offset_alg != NULL) {
                      ret_sync = new SKaMPIClockSync(offset_alg);
                  } else {
                      ZF_LOGE("problem with format of skampi clock offset alg");
                  }
              } else {
                  ZF_LOGE("format error sync alg '%s'", sync_alg.c_str());
              }
          } else if (sync_alg == "prop") {
              ret_sync = new ClockPropagationSync();
          } else {
              ZF_LOGE("unknown clock sync alg '%s'", sync_alg.c_str());
          }
      }

  } else {
      if (rank == 0) {
          ZF_LOGE("parameter '%s' not found, using default", param_name);
      }
  }

  return ret_sync;
}


