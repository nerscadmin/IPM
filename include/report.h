

#ifndef REPORT_H_INCLUDED
#define REPORT_H_INCLUDED

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "ipm_types.h"
#include "hashtable.h"
#include "hashkey.h"


/* global (across-tasks) statistics */
typedef struct
{
  /* have the activity recorded explicitly here so that we 
     can do a qsort on an array of gstat_t records */
  int            activity;
  double         dmin, dmax, dsum;
  IPM_COUNT_TYPE nmin, nmax, nsum;
} gstats_t;

#define GSTATS_CLEAR(gs_)			\
  gs_.dmin=0.0; gs_.dmax=0.0; gs_.dsum=0.0;	\
  gs_.nmin=0;   gs_.nmax=0;   gs_.nsum=0;		

#define GSTATS_SET(gs_,dval_,nval_)			\
  gs_.dmin=dval_; gs_.dmax=dval_; gs_.dsum=dval_;	\
  gs_.nmin=nval_; gs_.nmax=nval_; gs_.nsum=nval_;		

#define GSTATS_ADD(gs_,dval_,nval_)			\
  gs_.dmin+=dval_; gs_.dmax+=dval_; gs_.dsum+=dval_;	\
  gs_.nmin+=nval_; gs_.nmax+=nval_; gs_.nsum+=nval_;		

  
/* compute global statistics for hent data */
void gstats_hent(ipm_hent_t hent, gstats_t *global);

/* compute global statistics for double values */
void gstats_double(double val, gstats_t *global);

/* compute global statistics for counts */
void gstats_count(IPM_COUNT_TYPE count, gstats_t *global);


/* flags for banner printing */
#define BANNER_FULL             (0x00000001UL <<  0)
#define BANNER_HAVE_MPI         (0x00000001UL <<  1)
#define BANNER_HAVE_POSIXIO     (0x00000001UL <<  2)
#define BANNER_HAVE_OMP         (0x00000001UL <<  3)
#define BANNER_HAVE_CUDA        (0x00000001UL <<  4)
#define BANNER_HAVE_CUBLAS      (0x00000001UL <<  5)
#define BANNER_HAVE_CUFFT       (0x00000001UL <<  6)
#define BANNER_HAVE_ENERGY      (0x00000001UL <<  7)

#define XML_CLUSTERED           (0x00000001UL <<  8)
#define XML_RELATIVE_RANKS      (0x00000001UL <<  9)


typedef struct 
{
  char name[MAXSIZE_REGLABEL];
  char nesting[MAXNUM_REGNESTING][MAXSIZE_REGLABEL];
  int  valid;
  
  gstats_t gflops;
  gstats_t wallt;   /* wallclock time */
  gstats_t mpi;     /* times and counts for MPI */
  gstats_t mpip;    /* percent MPI */
  gstats_t pio;     /* times and counts for POSIX-IO */
  gstats_t piop;    /* percent POSIX-IO */
  gstats_t omp;     /* times and counts for OpenMP */
  gstats_t ompp;    /* percent OpenMP */
  gstats_t ompi;    /* idle time in OpenMP */

  gstats_t cuda;     /* times and counts for CUDA */
  gstats_t cudap;    

  gstats_t cublas;   /* times and counts for CUBLAS */
  gstats_t cublasp;    

  gstats_t cufft;    /* times and counts for CUFFT */
  gstats_t cufftp;    

  gstats_t energy;
  
  /* --- everything below is only 
     touched if (flags&BANNER_FULL) --- */
  
  /* global hashtable statistics used in full banner */
  gstats_t fullstats[MAXSIZE_CALLTABLE];
} regstats_t;



typedef struct
{
  unsigned long  flags;       
  struct timeval tstart, tstop;
  char cmdline[MAXSIZE_CMDLINE];
  char hostname[MAXSIZE_HOSTNAME];
  int ntasks, nhosts, nthreads;
  int nregions;
  gstats_t procmem;
  gstats_t gflop;
  gstats_t energy;

  char* calltable[MAXSIZE_CALLTABLE];
  
  regstats_t app;
  regstats_t regions[MAXNUM_REGIONS];
} banner_t;


#define BANNER_SET_NTASKS(b_, n_)    b_.ntasks=n_;
#define BANNER_SET_NHOSTS(b_, n_)    b_.nhosts=n_;
#define BANNER_SET_NTHREADS(b_, n_)  b_.nthreads=n_;
#define BANNER_SET_NREGIONS(b_, n_)  b_.nregions=n_;

#define BANNER_SET_CMDLINE(b_, s_)		\
  strncpy(b_.cmdline,s_,MAXSIZE_CMDLINE);	\
  b_.cmdline[MAXSIZE_CMDLINE-1]=0;

#define BANNER_SET_HOSTNAME(b_, s_)		\
  strncpy(b_.hostname,s_,MAXSIZE_HOSTNAME);	\
  b_.hostname[MAXSIZE_HOSTNAME-1]=0;

#define BANNER_SET_TSTART(b_, t_)		\
  banner.tstart.tv_sec = t_.tv_sec;		\
  banner.tstart.tv_usec = t_.tv_usec;

#define BANNER_SET_TSTOP(b_, t_)		\
  banner.tstop.tv_sec = t_.tv_sec;		\
  banner.tstop.tv_usec = t_.tv_usec;




void report_set_filename();

int report_xml_atroot(unsigned long flags);
int report_xml_mpiio(unsigned long flags);

void ipm_banner(FILE *f);

void ipm_print_banner(FILE *f, banner_t *data);

int ipm_printf(void *ptr, const char* format, ...);


#endif /* REPORT_H_INCLUDED */
