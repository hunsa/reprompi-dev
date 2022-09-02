# ReproMPI Benchmark (Development Version)


## Introduction

The ReproMPI Benchmark is a tool designed to accurately measure the
run-time of MPI blocking collective operations.  It provides multiple
process synchronization methods and a flexible mechanism for
predicting the number of measurements that are sufficient to obtain
statistically sound results.


## Installation

- Prerequisites
  - an MPI library 
  - CMake (version >= 3.0)  
  - GSL libraries 

## Basic installation

```
  cd $BENCHMARK_PATH
  ./cmake .
  make
```

For specific configuration options check the *Benchmark Configuration* section.

## Running the ReproMPI Benchmark

The ReproMPI code is designed to serve two specific purposes:

## Benchmarking of MPI collective calls
The most common usage scenario of the benchmark is to specify an MPI
collective function to be benchmarked, a (list of) message sizes and
the *number of measurement repetitions* for each test, as in the
following example.

```
mpirun -np 4 ./bin/mpibenchmark --calls-list=MPI_Bcast,MPI_Allgather 
             --msizes-list=8,1024,2048  --nrep=10
```



## Command-line Options

### Common Options

  - `-h` print help
  - `-v` print run-times measured for each process
  - `--msizes-list`<values>= list of comma-separated message sizes in
    Bytes, e.g., `--msizes-list=10,1024`
  - `--msize-interval=min=<min>,max=<max>,step=<step>` list of power
    of 2 message sizes as an interval between $2^{min}$ and $2^{max}$,
    with $2^{step}$ distance between values, e.g., 
    `--msize-interval=min=1,max=4,step=1`
  - `--calls-list=<args>` list of comma-separated MPI calls to be
    benchmarked, e.g., `--calls-list=MPI_Bcast,MPI_Allgather`
  - `--root-proc=<process_id>` root node for collective operations     
  - `--operation=<mpi_op>` MPI operation applied by collective
    operations (where applicable), e.g., `--operation=MPI_BOR`.
    
    Supported operations: MPI_BOR, MPI_BAND, MPI_LOR, MPI_LAND,
    MPI_MIN, MPI_MAX, MPI_SUM, MPI_PROD 
  - `--datatype=<mpi_type>` MPI datatype used by collective
    operations, e.g., `--datatype=MPI_CHAR`.

    Supported datatypes: `MPI_CHAR`, `MPI_INT`, `MPI_FLOAT`, `MPI_DOUBLE`
  - `--shuffle-jobs` shuffle experiments before running the benchmark
  - `--params=k1:v1,k2:v2` list of comma-separated =key:value= pairs
    to be printed in the benchmark output.
  - `-f | --input-file=<path>` input file containing the list of
    benchmarking jobs (tuples of MPI function, message size, number of
    repetitions). It replaces all the other common options.
  
  
### Options Related to the Window-based Synchronization

  - `--window-size=<win>` window size in microseconds for Window-based synchronization


### Specific Options for the ReproMPI Benchmark

  - `--nrep=<nrep>` set number of experiment repetitions
  - `--summary=<args>` list of comma-separated data summarizing
    methods (mean, median, min, max), e.g., `--summary=mean,max`


## Supported Collective Operations:
### MPI Collectives

  - `MPI_Allgather`
  - `MPI_Allreduce`
  - `MPI_Alltoall`
  - `MPI_Barrier`
  - `MPI_Bcast`
  - `MPI_Exscan`
  - `MPI_Gather`
  - `MPI_Reduce`
  - `MPI_Reduce_scatter`
  - `MPI_Reduce_scatter_block`
  - `MPI_Scan`
  - `MPI_Scatter`

### Mockup Functions of Various MPI Collectives
  TODO

    

## Process Synchronization Methods

### MPI_Barrier
This is the default synchronization method enabled for the benchmark.

### Dissemination Barrier
To benchmark collective operations acorss multiple MPI libraries using
the same barrier implementation, the benchmark provides a
dissemination barrier that can replace the default MPI_Barrier to
synchronize processes.

To enable the dissemination barrier, the following flag has to be set
before compiling the benchmark (e.g., using the =ccmake= command).

```
ENABLE_BENCHMARK_BARRIER
```

Both barrier-based synchronization methods can alternatively use a
double barrier before each measurement.

```
ENABLE_DOUBLE_BARRIER
```


### Window-based Synchronization

The ReproMPI benchmark implements a window-based process
synchronization mechanism, which estimates the clock offset/drift of
each process relative to a reference process and then uses the
obtained global clocks to synchronize processes before each
measurement and to compute run-times.


### Timing procedure
  
  The MPI operation run-time is computed in a different manner
  depending on the selected clock synchronization method. If global
  clocks are available, the run-times are computed as the difference
  between the largest exit time and the first start time among all
  processes.

  If a barrier-based synchronization is used, the run-time of an MPI
  call is computed as the largest local run-time across all processes.

  However, the timing proceduce that relies on global clocks can be
  used in combination with a barrier-based synchronization when the
  following flag is enabled:


### Clock resolution

The =MPI_Wtime= cll is used by default to obtain the current time.
To obtain accurate measurements of short time intervals, the benchmark
can rely on the high resolution =RDTSC/RDTSCP= instructions (if they are
available on the test machines) by setting on of the following flags:
```
ENABLE_RDTSC
ENABLE_RDTSCP
```

Additionally, setting the clock frequency of the CPU is required to
obtain accurate measurements:
```
FREQUENCY_MHZ                    2300
```

The clock frequency can also be automatically estimated (as done by
the NetGauge tool) by enabling the following variable:
```
CALIBRATE_RDTSC
```

However, this method reduces the results accuracy and we advise to
manually set the highest CPU frequency instead. More details about
the usage of =RDTSC=-based timers can be found in our research
report.

## List of Compilation Flags

This is the full list of compilation flags that can be used to control
all the previously detailed configuration parameters.

```
 CALIBRATE_RDTSC                  OFF   
 COMPILE_BENCH_TESTS              OFF                 
 COMPILE_SANITY_CHECK_TESTS       OFF               
 ENABLE_BENCHMARK_BARRIER         OFF             
 ENABLE_DOUBLE_BARRIER            OFF             
 ENABLE_GLOBAL_TIMES              OFF             
 ENABLE_LOGP_SYNC                 OFF             
 ENABLE_RDTSC                     OFF             
 ENABLE_RDTSCP                    OFF           
 ENABLE_WINDOWSYNC_HCA            OFF            
 ENABLE_WINDOWSYNC_JK             OFF        
 ENABLE_WINDOWSYNC_SK             OFF      
 FREQUENCY_MHZ                    2300    
```
