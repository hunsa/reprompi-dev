//
// Created by Sascha on 11/3/22.
//

#include "comparer_factory.h"
#include "simple_comparer.h"
#include "ttest_comparer.h"
#include "detailed_ttest_comparer.h"
#include "grouped_ttest_comparer.h"

PGDataComparer* ComparerFactory::create_comparer(int comparer_id, std::string mpi_coll_name, int nnodes, int ppn) {
  PGDataComparer *comparer;
  switch(comparer_id) {
  case 0:
    comparer = new SimpleComparer(mpi_coll_name, nnodes, ppn);
    break;
  case 1:
    comparer = new DetailedTTestComparer(mpi_coll_name, nnodes, ppn);
    break;
  case 2:
    comparer = new TTestComparer(mpi_coll_name, nnodes, ppn);
    break;
  case 3:
    comparer = new GroupedTTestComparer(mpi_coll_name, nnodes, ppn);
    break;
  default:
    comparer = new GroupedTTestComparer(mpi_coll_name, nnodes, ppn);
    break;
  }
  return comparer;
}