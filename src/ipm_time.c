#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <sys/resource.h>

#include "ipm.h"
#include "ipm_time.h"
#include "ipm_debug.h"
#include "hashtable.h"

#ifdef HAVE_MPI
#include "GEN.calltable_mpi.h"
#endif

#ifdef HAVE_POSIXIO
#include "GEN.calltable_posixio.h"
#endif

#ifdef HAVE_OMPTRACEPOINTS
#include "mod_omptracepoints.h"
#endif

#ifdef HAVE_MPIIO
#include "GEN.calltable_mpiio.h"
#endif

#ifdef HAVE_CUDA
#include "GEN.calltable_cuda.h"
#endif

#ifdef HAVE_CUBLAS
#include "GEN.calltable_cublas.h"
#endif

#ifdef HAVE_CUFFT
#include "GEN.calltable_cufft.h"
#endif



double ipm_seconds_per_tick=1.0;

void ipm_time_init(int flags) 
{
#ifdef HAVE_RDTSC 
  double speed = 0.0;				
  char sbuffer[1024];				
  
  sprintf(sbuffer,"/proc/cpuinfo");		
  FILE* fp = fopen(sbuffer,"r");		
  if(fp){					
    while(fgets(sbuffer,1024,fp)){		
      if(!strncmp(sbuffer,"cpu MHz",7)){	
	char* p = strchr(sbuffer,':');		
	if(p){					
	  sscanf(++p,"%lf",&speed);		
	}					
	break;					
      }						
    }						
    fclose(fp);					
  }							
  ipm_seconds_per_tick= (1.0 / (speed * 1.0e6));	
#endif
}


double ipm_timestamp()
{
  double time;
  IPM_TIMESTAMP(time);
  return time;
}

double ipm_wtime()
{
  double time=0.0;
  static struct timeval tv;
  
  gettimeofday( &tv, NULL );
  time=IPM_TIMEVAL(tv);

  return time;
}

double ipm_utime()
{
  double time;
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);  
  time = ru.ru_utime.tv_sec+ru.ru_utime.tv_usec*1.0e-6;

  return time;
}


double ipm_stime()
{
  double time;
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);  
  time = ru.ru_stime.tv_sec+ru.ru_stime.tv_usec*1.0e-6;

  return time;
}


double ipm_mtime()
{
  double time;

#ifdef HAVE_MPI
  scanstats_t stats;
  stats.hent.t_tot=0.0;

  htable_scan_activity( ipm_htable, &stats,
			MPI_MINID_GLOBAL, MPI_MAXID_GLOBAL);

  time = stats.hent.t_tot;
#else
  time = 0.0;
#endif

  return time;
}

double ipm_iotime()
{
#ifdef HAVE_POSIXIO
  scanstats_t stats;
  stats.hent.t_tot=0.0;

  htable_scan_activity( ipm_htable, &stats,
			POSIXIO_MINID_GLOBAL, POSIXIO_MAXID_GLOBAL);
  return stats.hent.t_tot;
#endif

  return 0.0;
}

double ipm_omptime()
{
#ifdef HAVE_OMPTRACEPOINTS
  scanstats_t stats;
  stats.hent.t_tot=0.0;

  htable_scan_activity( ipm_htable, &stats,
			OMP_PARALLEL_ID_GLOBAL, OMP_PARALLEL_ID_GLOBAL );
  return stats.hent.t_tot;
#endif

  return 0.0;
}

double ipm_ompidletime()
{
#ifdef HAVE_OMPTRACEPOINTS
  scanstats_t stats;
  stats.hent.t_tot=0.0;

  htable_scan_activity( ipm_htable, &stats,
			OMP_IDLE_ID_GLOBAL, OMP_IDLE_ID_GLOBAL );
  return stats.hent.t_tot;
#endif

  return 0.0;
}


double ipm_mpiiotime()
{
#ifdef HAVE_MPIIO
  scanstats_t stats;
  stats.hent.t_tot=0.0;
    
  htable_scan_activity( ipm_htable, &stats,
			MPIIO_MINID_GLOBAL, MPIIO_MAXID_GLOBAL);

  return stats.hent.t_tot;
#endif /* HAVE_MPIIO */

  return 0.0;
}


double ipm_cudatime()
{
  double time=0.0;
#ifdef HAVE_CUDA
  scanstats_t stats;
  stats.hent.t_tot=0.0;
  
  htable_scan_activity( ipm_htable, &stats,
			CUDA_MINID_GLOBAL, CUDA_MAXID_GLOBAL);
  time = stats.hent.t_tot;
#endif /* HAVE_CUDA */
  return time;
}


double ipm_cublastime()
{
  double time=0.0;
#ifdef HAVE_CUBLAS
  scanstats_t stats;
  stats.hent.t_tot=0.0;
  
  htable_scan_activity( ipm_htable, &stats,
			CUBLAS_MINID_GLOBAL, CUBLAS_MAXID_GLOBAL);
  time = stats.hent.t_tot;
#endif /* HAVE_CUBLAS */
  return time;
}


double ipm_cuffttime()
{
  double time=0.0;
#ifdef HAVE_CUFFT
  scanstats_t stats;
  stats.hent.t_tot=0.0;
  
  htable_scan_activity( ipm_htable, &stats,
			CUFFT_MINID_GLOBAL, CUFFT_MAXID_GLOBAL);
  time = stats.hent.t_tot;
#endif /* HAVE_CUFFT */
  return time;
}
