//
// Created by Max on 11/16/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H
#define REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H

#include "pgdata.h"
#include "comparer/comparer_factory.h"
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>
#include <iostream>
#include <fstream>

class PGDataPrinter {

private:
  void add_data_storage(std::string data);

protected:
  int comparer_type;
  std::string output_directory;
  bool detailed;
  std::vector<std::string> mpi_coll_names;
  int nnodes;  // number of nodes
  int ppn;     // number of processes per node
  bool verbose;
  std::unordered_map<std::string, PGData *> mockup2data;
  double barrier_time_s = -1.0;
  std::string data_storage;

public:

  PGDataPrinter(
      int comparer_type,
      std::string output_directory,
      bool detailed,
      int nnodes,
      int ppn,
      bool verbose
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