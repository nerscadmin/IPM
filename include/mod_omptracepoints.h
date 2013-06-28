

#ifndef OMPTRACEPOINTS_H_INCLUDED
#define OMPTRACEPOINTS_H_INCLUDED

#include "ipm_modules.h"
 
#define OMP_OFFSET        180

#define OMP_PARALLEL_ID_GLOBAL   (0 + OMP_OFFSET)
#define OMP_IDLE_ID_GLOBAL       (1 + OMP_OFFSET)

#define OMP_MINID_GLOBAL  OMP_PARALLEL_ID_GLOBAL
#define OMP_MAXID_GLOBAL  OMP_IDLE_ID_GLOBAL


typedef struct 
{
  unsigned nenter; /* number of times in parallel region  */
  double tenter;   /* timestamp of last enter */
  double twork;    /* time in parallel region doing work */
  double tidle;    /* time in parallel region being idle */
  /* tpar = twork+tidle time in parallel region */
} ompstats_t;

typedef struct ompdata
{
  double omptime;
  double omptime_e;
  double idletime;
  double idletime_e;
} ompdata_t;

extern int nthreads; /* number of threads in par region */
extern int maxthreads; /* maximum number of threads encountered */


int mod_omptracepoints_init(ipm_mod_t *mod, int flags);

int mod_omptracepoints_output(ipm_mod_t *mod, int flags);


#endif /* OMPTRACEPOINTS_H_INCLUDED */
