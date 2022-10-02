
#ifndef REPROMPI_DEV_SRC_PGCHECK_UTILS_ARGV_MANAGER_H
#define REPROMPI_DEV_SRC_PGCHECK_UTILS_ARGV_MANAGER_H

#include <string>
#include <vector>

namespace argv {

void compose_argv_vector(std::string prog_name,
                         std::string reprompi_params,
                         std::vector <std::string> &pgtunelib_params,
                         std::vector <std::string> &argv_vector);


void convert_vector_to_argv_cstyle(std::vector <std::string> &argv_vector,
                                   int *argc_out, char ***argv_out);

void free_argv_cstyle(int argc_test, char **argv_test);

}

#endif //REPROMPI_DEV_SRC_PGCHECK_UTILS_ARGV_MANAGER_H
