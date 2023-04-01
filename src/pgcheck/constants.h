// Copyright 2023 Hagn Maximilian
// Created by maximilian on 28.03.23.
//

#ifndef SRC_PGCHECK_CONSTANTS_H_
#define SRC_PGCHECK_CONSTANTS_H_

#include <vector>
#include <string>

namespace CONSTANTS {

  constexpr auto ROOT_PROCESS_RANK = 0;
  constexpr auto FIRST_VIOLATION_COMPARER = 2;
  constexpr auto LAST_VIOLATION_COMPARER = 6;
  constexpr int SUCCESS = 0;
  constexpr int FAILURE = -1;
  constexpr double NO_BARRIER_TIME_VALUE = -1.0;
  const std::vector <std::string> COMPARER_NAMES = {"simple",
                                                    "abs_runtime",
                                                    "rel_runtime",
                                                    "violation",
                                                    "detailed_violation",
                                                    "grouped_violation",
                                                    "raw"};
}  // namespace CONSTANTS


#endif  // SRC_PGCHECK_CONSTANTS_H_
