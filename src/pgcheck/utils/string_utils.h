//
// Created by Sascha on 10/2/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_UTILS_STRING_UTILS_H
#define REPROMPI_DEV_SRC_PGCHECK_UTILS_STRING_UTILS_H

#include <string>
#include <vector>

std::string ltrim(const std::string &s);
std::string rtrim(const std::string &s);
std::string trim(const std::string &s);
std::vector<std::string> string_split(std::string s, char delimiter);

#endif //REPROMPI_DEV_SRC_PGCHECK_UTILS_STRING_UTILS_H
