
#ifndef PERFDATA_H_INCLUDED
#define PERFDATA_H_INCLUDED

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "ipm.h"
#include "ipm_sizes.h"
#include "ipm_types.h"
#include "regstack.h"

#ifdef HAVE_POSIXIO
#include "mod_posixio.h"
#endif

#ifdef HAVE_CUDA
#include "mod_cuda.h"
#endif

#ifdef HAVE_CUBLAS
#include "mod_cublas.h"
#endif

#ifdef HAVE_CUFFT
#include "mod_cufft.h"
#endif

#ifdef HAVE_OMPTRACEPOINTS
#include "mod_omptracepoints.h"
#endif

typedef struct jobdata 
{
  char jobid[MAXSIZE_JOBID];
  
  /* which executables in the job to report on and log?  */
  int ntargets;
  
  /* list by name of executable (name contains string)   */
  char *targets_exec[MAXSIZE_CMDLINE];
  
  /* any executable exceeding tmin seconds of execution  */
  double target_tmin;
} jobdata_t;


typedef struct taskdata 
{
  pid_t pid;
  int taskid, ntasks;
  unsigned long long int flags;

  struct timeval t_start;
  struct timeval t_stop;

  char hostname[MAXSIZE_HOSTNAME];

  /* wallclock, system, user, mpi, io, omp*/
  double wtime, stime, utime, mtime, iotime, omptime, ompidletime; 

  double procmem;

  int nhosts;
  int par_env;
  char user[MAXSIZE_USERNAME];
  char allocation[MAXSIZE_ALLOCATIONNAME];
  char jobid[MAXSIZE_JOBID];
  char mach_name[MAXSIZE_MACHNAME];
  char mach_info[MAXSIZE_MACHINFO];
  
  char exec_cmdline[MAXSIZE_CMDLINE];
  char exec_realpath[MAXSIZE_CMDLINE];
  char exec_md5sum[MAXSIZE_CMDLINE];
  
  char appname[MAXSIZE_CMDLINE];

  char fname[MAXSIZE_FILENAME];
  char logdir[MAXSIZE_FILENAME];

#if defined(HAVE_POSIXIO_TRACE) || defined(HAVE_MPI_TRACE) || defined(HAVE_OMP_TRACE)
  FILE *tracefile;
  int tracestate;
#endif
#if defined(HAVE_SNAP)
  double snap_last, snap_period;
  FILE *snap_fh;
  char snap_fname[MAXSIZE_FILENAME];
#endif

  struct region *rstack;

#ifdef HAVE_POSIXIO
  iodata_t iodata[MAXNUM_REGIONS];
#endif

#ifdef HAVE_CUDA
  cudadata_t cudadata[MAXNUM_REGIONS];
#endif 

#ifdef HAVE_CUBLAS
  cublasdata_t cublasdata[MAXNUM_REGIONS];
#endif

#ifdef HAVE_CUFFT
  cufftdata_t cufftdata[MAXNUM_REGIONS];
#endif

#ifdef HAVE_OMPTRACEPOINTS
  ompdata_t ompdata[MAXNUM_REGIONS];
#endif

} taskdata_t;

typedef struct threaddata 
{
  int tid,threadid,nthreads;
} threaddata_t;


extern taskdata_t task;

void taskdata_init(taskdata_t *t);

#ifdef HAVE_SNAP
#define IPM_SNAP {							\
    if(task.snap_period && tstop - task.snap_last >= task.snap_period) { \
      ipm_state=STATE_NOTACTIVE;					\
      sprintf(task.snap_fname,"/tmp/ipm/%s_%d",task.fname,task.taskid); \
      task.snap_fh = fopen(task.snap_fname,"w");			\
      chmod(task.snap_fname,S_IWOTH|S_IROTH|S_IWGRP|S_IRGRP|S_IWUSR|S_IRUSR|S_IXUSR|S_IXGRP|S_IXOTH);					\
      ipm_region_end(&ipm_app);						\
      xml_task(task.snap_fh,&task,ipm_htable);				\
      ipm_region_begin(&ipm_app);					\
      fclose(task.snap_fh);						\
      task.snap_last = tstop;						\
      ipm_state=STATE_ACTIVE;						\
    }									\
  }
#endif


#endif /* PERFDATA_H_INCLUDED */
