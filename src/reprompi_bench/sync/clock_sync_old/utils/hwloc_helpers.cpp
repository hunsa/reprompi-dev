#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>


#include <hwloc.h>


int get_socket_id(void) {
  unsigned i;
  hwloc_topology_t topology;
  hwloc_obj_t obj;
  hwloc_bitmap_t set;
  int err;
  int my_rank;
  unsigned int socket_id;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  /* Allocate and initialize topology object. */
  hwloc_topology_init(&topology);

  /* Perform the topology detection. */
  hwloc_topology_load(topology);

  /* retrieve the CPU binding of the current entire process */
  set = hwloc_bitmap_alloc();
  if (!set) {
    fprintf(stderr, "[get_socket_id] failed to allocate a bitmap\n");
    hwloc_topology_destroy(topology);
    return -1;
  }
  err = hwloc_get_cpubind(topology, set, HWLOC_CPUBIND_PROCESS);
  if (err < 0) {
    fprintf(stderr, "[get_socket_id] failed to get cpu binding\n");
    hwloc_bitmap_free(set);
    hwloc_topology_destroy(topology);
    return -1;
  }

  // check that all cores where a rank can be placed belong to the same socket
  socket_id = -1;
  err = 0;
  hwloc_bitmap_foreach_begin(i, set)
        { // one process may be allowed on multiple cores
          for (obj = hwloc_get_pu_obj_by_os_index(topology, i); obj; obj = obj->parent) {
            if (hwloc_compare_types(obj->type, HWLOC_OBJ_SOCKET) == 0) {
              if (socket_id >= 0 && socket_id != obj->logical_index) { // the current rank can be placed on multiple sockets => return error
                fprintf(stderr, "[get_socket_id] Rank %d can be placed on multiple sockets (e.g., %d and %d)\n",
                    my_rank, socket_id, obj->logical_index);
                err = 1;
              } else {
                socket_id = obj->logical_index;
              }
              break; // found the socket level
            }
          }
          if (err == 1) {
            break;
          }
          //printf("Rank %d: %s=%d\n", my_rank, hwloc_obj_type_string(obj->type), obj->logical_index);
        }hwloc_bitmap_foreach_end();
  hwloc_bitmap_free(set);

  /* Destroy topology object. */
  hwloc_topology_destroy(topology);

  if (err == 1) {
    return -1;
  }
  return socket_id;
}

