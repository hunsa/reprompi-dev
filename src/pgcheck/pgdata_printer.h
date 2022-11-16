//
// Created by Max on 11/16/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H
#define REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H

#include "pgdata.h"
#include "comparer/comparer_factory.h"
#include "pgcheck_options.h"
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>
#include <iostream>
#include <fstream>

class PGDataPrinter {

private:
  void add_data_storage(std::string data);
  std::string pgdata_to_string(PGDataResults data_result);
  std::string pgdata_to_csv_string(PGDataResults data_result);

protected:
  PGCheckOptions * options;
  int nnodes;
  int ppn;
  double barrier_time_s = -1.0;
  std::vector<std::string> mpi_coll_names;
  std::unordered_map<std::string, PGData *> mockup2data;

public:

  PGDataPrinter(
      PGCheckOptions * options,
      int nnodes,
      int ppn
  );

  int print_collective();

  void print_summary();

  void println_to_cerr(std::string message);

  void println_to_cout(std::string message);

  void add_dataframe_mockup(std::string mockup_name, PGData *data);

  void add_mpi_coll_name(std::string mpi_coll_name);

  void set_barrier_time(double time_s);


};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H
