
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ipm_core.h"

int ipm_get_procmem(double *bytes) 
{
  FILE *fh;
  int proc_ret;
  char proc_var[80];
  char *cp;
  long long int ibytes;
  
#if defined(OS_LINUX)
#ifndef LINUX_XT3
#define PROCMEM_LINUX_PROC
#endif
#endif
  
#if defined (OS_AIX) 
#define PROCMEM_GETRUSAGE
#endif

#ifdef PROCMEM_LINUX_PROC
#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif
  
/* Old logic was max of VmPeak and VmHWM - for some reason */
/* VmPeak is now much too large by ~3GB - so cade is now switched to */
/* VmHWM. Nick Dec 15 2010 */
  *bytes=0.0;
  fh = fopen("/proc/self/status","r");
  while(!feof(fh)) {
    fgets(proc_var,80,fh);
/*    cp = strstr(proc_var,"VmPeak:");
    if (cp) {sscanf(cp, "VmPeak:"" %llu",&ibytes );
      *bytes=max(*bytes,ibytes);
    } 
*/
    cp = strstr(proc_var,"VmHWM:");
    if (cp) {sscanf(cp, "VmHWM:"" %llu",&ibytes );
      *bytes=max(*bytes,ibytes);
    }
  }
  fclose(fh);
  *bytes *= 1024.0;
  
#elif defined (PROCMEM_GETRUSAGE)
  
  getrusage(RUSAGE_SELF,&task.ru_SELF_curr); 
  getrusage(RUSAGE_CHILDREN,&task.ru_CHILD_curr); 
  *bytes =    (task.ru_SELF_curr.ru_maxrss + 
	       task.ru_CHILD_curr.ru_maxrss )*1024.0;
  
#else 
  *bytes = 0.0;
#endif
  return IPM_OK;
}

#if 0

/* this is how we would return the memory currently in use */
static int ipm_get_procmem_now(double *bytes) {
 FILE *fh;
 int pagesize=getpagesize();
 fh = fopen("/proc/self/statm", "r");
  if(fscanf(fh,"%lf", bytes)!=1) {
   printf("IPM: %d error in ipm_get_procmem\n", task.mpi_rank);
  }
 fclose(fh);
 *bytes *= pagesize;
 return 0;
}

/* FIXME - unimplemented */
static int ipm_get_virtmem(double *bytes) {
 return 0;
}

#endif



#ifdef UTEST_MEMUSAGE

int main(int argc, char* argv[])
{
  double before, after;
  double *mem;

  ipm_get_procmem(&before);

  mem = calloc( 1000*1000, sizeof(double) );
  
  ipm_get_procmem(&after);

    fprintf(stderr, "Testing ipm_get_procmem... %f %f %f\n", 
	  before, after, after-before);
 
}

#endif /* UTEST_MEMUSAGE */
