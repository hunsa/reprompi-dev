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

#ifndef REPROMPIB_PARSE_TIMING_OPTIONS_H_
#define REPROMPIB_PARSE_TIMING_OPTIONS_H_


typedef enum reprompi_timing_method {
  REPROMPI_RUNT_MAX_OVER_LOCAL_RUNTIME = 0,
  REPROMPI_RUNT_GLOBAL_TIMES
  //REPROMPI_RUNT_MAX_OVER_LOCAL_AVG
} reprompib_timing_method_t;


void reprompib_parse_timing_options(reprompib_timing_method_t* runtime_type, int argc, char** argv);

const char* reprompib_get_timing_method_name(reprompib_timing_method_t t);


#endif /* REPROMPIB_PARSE_TIMING_OPTIONS_H_ */
