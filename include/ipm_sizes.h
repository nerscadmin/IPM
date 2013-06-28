
#ifndef SIZES_H_INCLUDED
#define SIZES_H_INCLUDED

/* other prime number hash table sizes:
   
   #define MAXSIZE_HASH              4049
   #define MAXSIZE_HASH              8093
   #define MAXSIZE_HASH             16301
   #define MAXSIZE_HASH             32573
   #define MAXSIZE_HASH             65437
   
*/

#define MAXSIZE_HASH             65437

#ifdef HAVE_KEYHIST
#define MAXSIZE_XHASH            32573
#endif 


#define MAXSIZE_HOSTNAME         16
#define MAXSIZE_USERNAME         16
#define MAXSIZE_ALLOCATIONNAME   16
#define MAXSIZE_JOBID            32
#define MAXSIZE_MACHNAME         32
#define MAXSIZE_MACHINFO         32
#define MAXSIZE_REGLABEL         32
#define MAXSIZE_CMDLINE          4096
#define MAXSIZE_FILENAME         256
#define MAXNUM_REGIONS           256
#define MAXNUM_REGNESTING         32


#define MAXNUM_MODULES           16

/* module MPI */
#define MAXNUM_MPI_OPS           16
#define MAXNUM_MPI_TYPES         64

/* module callpath */
#define MAXSIZE_CALLSTACKDEPTH   30 
#define MAXSIZE_CALLTABLE        1024
#define MAXSIZE_CALLLABEL        64
#define MAXNUM_CALLSITES         8192

/* module keyhist */
#define MAXSIZE_CYCLE            128
#define MAXNUM_CYCLES            128

/* module papi */
#define MAXNUM_PAPI_EVENTS      16
#define MAXNUM_PAPI_COUNTERS    8
#define MAXNUM_PAPI_COMPONENTS  8 
#define MAXSIZE_PAPI_EVTNAME    32

/* module omptracepoints */
#define MAXNUM_THREADS          128


#endif /* IPM_SIZES_INCLUDED */
