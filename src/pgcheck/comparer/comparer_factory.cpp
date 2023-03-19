//
// Created by Sascha on 11/3/22.
//

#include "comparer_factory.h"
#include "simple_comparer.h"
#include "violation/violation_comparer.h"
#include "violation/detailed_violation_comparer.h"
#include "violation/grouped_violation_comparer.h"
#include "runtime/abs_runtime_comparer.h"
#include "runtime/rel_runtime_comparer.h"
#include "raw/raw_comparer.h"

PGDataComparer *
ComparerFactory::create_comparer(int comparer_id, int test_type, std::string mpi_coll_name, int nnodes, int ppn) {
  PGDataComparer *comparer;
  switch (comparer_id) {
    case 0:
      comparer = new SimpleComparer(mpi_coll_name, nnodes, ppn);
      break;
    case 1:
      comparer = new AbsRuntimeComparer(mpi_coll_name, nnodes, ppn);
      break;
    case 2:
      comparer = new RelRuntimeComparer(mpi_coll_name, nnodes, ppn);
      break;
    case 3:
      comparer = new ViolationComparer(test_type, mpi_coll_name, nnodes, ppn);
      break;
    case 4:
      comparer = new DetailedViolationComparer(test_type, mpi_coll_name, nnodes, ppn);
      break;
    case 5:
      comparer = new GroupedViolationComparer(test_type, mpi_coll_name, nnodes, ppn);
      break;
    default:
    case 6:
      comparer = new RawComparer(mpi_coll_name, nnodes, ppn);
      break;
  }
  return comparer;
}