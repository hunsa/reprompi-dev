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
  /**
   * @return table formatted for txt or console as string
   */
  std::string table_to_clear_string(PGDataTable data_result);

  /**
   * @return table formatted for csv file as string
   */
  std::string table_to_csv_string(PGDataTable data_result);

protected:
  int nnodes;
  int ppn;
  double barrier_time_s = -1.0;
  std::vector <std::string> mpi_coll_names;
  std::unordered_map<std::string, PGData *> mockup2data;
  PGCheckOptions *options;
  PGDataTable merged_table;

public:
  PGDataPrinter(int nnodes, int ppn, PGCheckOptions *options);

  /**
   * prints results from collective as txt or csv to file or console
   */
  int print_collective();

  /**
   * prints results from collective as txt or csv to file or console
   */
  void print_summary();

  void println_to_cerr(std::string message);

  void println_to_cout(std::string message);

  void add_dataframe_mockup(std::string mockup_name, PGData *data);

  void add_mpi_coll_name(std::string mpi_coll_name);

  void set_barrier_time(double time_s);


};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H
