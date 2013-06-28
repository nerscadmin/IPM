
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#ifdef HAVE_MPI
#include <mpi.h>
#endif 

#include "ipm.h"
#include "config.h"
#include "ipm_core.h"
#include "ipm_env.h"
#include "ipm_sizes.h"
#include "ipm_time.h"
#include "regstack.h"
#include "hashtable.h"
#include "report.h"
#include "machtopo.h"
#include "memusage.h"
#include "ipm_modules.h"
#include "jobdata.h"
#include "ipm_time.h"

#ifdef HAVE_POSIXIO
#include "mod_posixio.h"
#endif

#ifdef HAVE_MPI
#include "mod_mpi.h"
#endif

#ifdef HAVE_CALLPATH
#include "mod_callpath.h"
#endif 

#ifdef HAVE_KEYHIST
#include "mod_keyhist.h"
#endif 

#ifdef HAVE_PAPI
#include "mod_papi.h"
#endif

#ifdef HAVE_SELFMONITOR
#include "mod_selfmonitor.h"
#endif

#ifdef HAVE_PROCCTRL
#include "mod_procctrl.h"
#endif

#ifdef HAVE_CLUSTERING
#include "mod_clustering.h"
#endif

#ifdef HAVE_MPIIO
#include "mod_mpiio.h"
#endif

#ifdef HAVE_OMPTRACEPOINTS
#include "mod_omptracepoints.h"
#endif

#ifdef HAVE_CUDA
#include "mod_cuda.h"
#endif

#ifdef HAVE_CUFFT
#include "mod_cufft.h"
#endif

#ifdef HAVE_CUBLAS
#include "mod_cublas.h"
#endif

int ipm_state=STATE_NOTINIT;

void ipm_atexit_handler();
void ipm_sig_handler(int sig);
void ipm_write_profile_log();

int mod_selfmonitor_output(struct ipm_module* mod, int flags);
int mod_selfmonitor_init(struct ipm_module* mod, int flags);

int ipm_init(int flags) 
{
  int i, state, rv;
  double t_init;
  char cmdline[MAXSIZE_CMDLINE];
  char realpath[MAXSIZE_CMDLINE];
  char *target;

  state=ipm_state;
  ipm_state=STATE_IN_INIT;

  /* check if IPM_TARGET is set and if it is, 
     only monitor matching processes */
  target=getenv("IPM_TARGET");
  ipm_get_exec_cmdline(cmdline,realpath);

  /* IPM_TARGET specifies string that has to appear in cmdline */
  if( target && target[0]!='!' && !strstr(cmdline,target) ) {
    ipm_state=STATE_NOTACTIVE;
    return IPM_OK;
  }

  /* IPM_TARGET specifies string that must not appear in cmdline */
  if( target && target[0]=='!' && strstr(cmdline,target) ) {
    ipm_state=STATE_NOTACTIVE;
    return IPM_OK;
  }

  ipm_time_init(flags);

  rstack_init(flags);

  t_init = ipm_wtime();
  taskdata_init(&task);
  htable_init(ipm_htable);

  /* need to get env variables before modules init */
  ipm_get_env();

  /* init local data structures */
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    ipm_module_init( &(modules[i]) );
  }

  /* --- initialize modules --- */

#ifdef HAVE_MPI
  modules[IPM_MODULE_MPI].init=mod_mpi_init;
#endif 
  
#ifdef HAVE_POSIXIO
  modules[IPM_MODULE_POSIXIO].init=mod_posixio_init;
#endif   
  
#ifdef HAVE_MPIIO
  modules[IPM_MODULE_MPIIO].init=mod_mpiio_init;
#endif
  
#ifdef HAVE_CALLPATH
  modules[IPM_MODULE_CALLPATH].init=mod_callpath_init;
#endif
  
#ifdef HAVE_KEYHIST
  modules[IPM_MODULE_KEYHIST].init=mod_keyhist_init;
#endif

#ifdef HAVE_PAPI
  modules[IPM_MODULE_PAPI].init=mod_papi_init;
#endif

#ifdef HAVE_SELFMONITOR
  modules[IPM_MODULE_SELFMONITOR].init=mod_selfmonitor_init;
#endif

#ifdef HAVE_PROCCTRL
  modules[IPM_MODULE_PROCCTRL].init=mod_procctrl_init;
#endif

#ifdef HAVE_CLUSTERING
  modules[IPM_MODULE_CLUSTERING].init=mod_clustering_init;
#endif

#ifdef HAVE_OMPTRACEPOINTS
  modules[IPM_MODULE_OMPTRACEPOINTS].init=mod_omptracepoints_init;
#endif

#ifdef HAVE_CUDA
  modules[IPM_MODULE_CUDA].init=mod_cuda_init;
#endif   

#ifdef HAVE_CUFFT
  modules[IPM_MODULE_CUFFT].init=mod_cufft_init;
#endif   

#ifdef HAVE_CUBLAS
  modules[IPM_MODULE_CUBLAS].init=mod_cublas_init;
#endif   


  /* TODO: handle errors in module initialization, set 
     ipm_state to STATE_ERROR */
  
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    if( modules[i].init ) { /* call init function if it is set */
      rv=modules[i].init(&(modules[i]), flags); 
      if(rv!=IPM_OK) {
	IPMERR("Error initializing module %d (%s), error %d\n", 
	       i, modules[i].name?modules[i].name:"", rv);
      }

#ifdef HAVE_MPI
      if( i==IPM_MODULE_POSIXIO )
	modules[i].state=STATE_NOTACTIVE;
#endif
    }
  }
  
  /* --- done initializing modules --- */


#ifdef DELAYED_MPI_FINALIZE
  rv = atexit(ipm_atexit_handler);
  if(!rv) {
    task.flags|=FLAG_USING_ATEXIT;
  } else {
    IPMERR("Error installing atexit() handler\n");
    task.flags&=~FLAG_USING_ATEXIT;
  }
#endif 

  signal(SIGXCPU, ipm_sig_handler);
  signal(SIGTERM, ipm_sig_handler); 
  signal(SIGABRT, ipm_sig_handler); 

#ifdef HAVE_SELFMONITOR
  ipm_selfmon.t_init = ipm_wtime()-t_init;
#endif 

  /* this should be close to user code */
  ipm_region(1, "ipm_main");
  ipm_region_begin(&ipm_app);

  ipm_state=STATE_ACTIVE;
  return IPM_OK;
}


void ipm_atexit_handler()
{
  int isinit;

  IPMDBG("in ipm_atexit_handler()\n");

  ipm_finalize(0);

  IPMDBG("after ipm_finalize()\n");
  
#if defined(HAVE_MPI) && defined(DELAYED_MPI_FINALIZE)
  isinit=0;
  PMPI_Initialized(&isinit);
  if( isinit ) PMPI_Finalize();
#endif
}

void ipm_sig_handler(int sig) 
{
  int isinit;
  
  IPMDBG("In ipm_sig_handler() sig=%d\n", sig);
  if(sig == SIGTERM || sig == SIGXCPU || sig==SIGABRT) {
    ipm_finalize(0);
#if defined(HAVE_MPI) && defined(DELAYED_MPI_FINALIZE)
    isinit=0;
    PMPI_Initialized(&isinit);
    if( isinit ) PMPI_Finalize();
#endif
  }
}


int ipm_finalize(int flags) 
{
  int rv, i;

  if(ipm_state!=STATE_ACTIVE && 
     ipm_state!=STATE_NOTACTIVE ) {
    IPMERR("ipm_finalize() called with ipm_state=%d\n", ipm_state);
    return IPM_EOTHER;
  }

  ipm_state = STATE_IN_FINALIZE;

  /* this should be close to user code */
  ipm_region_end(&ipm_app);
  ipm_region(-1, "ipm_main");

  /* update global timing statistics */
  gettimeofday( &(task.t_stop), 0);
  task.wtime   = ipm_wtime()  -task.wtime;
  task.utime   = ipm_utime()  -task.utime;
  task.stime   = ipm_stime()  -task.stime;
  task.mtime   = ipm_mtime()  -task.mtime;
  task.iotime  = ipm_iotime() -task.iotime;
  task.omptime = ipm_omptime()-task.omptime;

  ipm_get_procmem(&(task.procmem));
  task.procmem /= (1024.0*1024.0*1024.0);

#ifdef HAVE_SELFMONITOR
  ipm_selfmon.t_finalize = ipm_wtime()-ipm_selfmon.t_finalize;
#endif 

#ifdef HAVE_PAPI
  //rstack_adjust_ctrs();
#endif

  /* write out banner report */  
  if( !(task.flags&FLAG_REPORT_NONE) ) {
    fflush(stdout); 
    ipm_banner(stdout);
  }

#if defined(HAVE_MPI) && defined(HAVE_CALLPATH)
  ipm_unify_callsite_ids();
#endif
  
  /* call output routine for each module */
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    if( i==IPM_MODULE_SELFMONITOR ||
	i==IPM_MODULE_MPI) 
      continue;

    if( modules[i].output!=0 ) {
      IPMDBG("calling output() for module %d\n", i);
      rv = modules[i].output(&(modules[i]), flags);
    }
  }

  ipm_write_profile_log();

  rstack_cleanup(ipm_rstack); 

#ifdef HAVE_SELFMONITOR
  mod_selfmonitor_output(&(modules[IPM_MODULE_SELFMONITOR]), flags);
#endif


  /* call finalization routine for each module */
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    
    if( modules[i].finalize!=0 ) {
      IPMDBG("calling finalize() for module %d\n", i);
      rv = modules[i].finalize(&(modules[i]), flags);
    }
  }  
  
  /* TODO: check errors in modules */
  ipm_state=STATE_FINALIZED;

  return IPM_OK;
}



void ipm_write_profile_log()
{
  unsigned long reportflags;
  
  reportflags=0;
#ifdef HAVE_CLUSTERING
  reportflags|=XML_CLUSTERED;
  reportflags|=XML_RELATIVE_RANKS;
#endif  

#ifdef HAVE_MPI 
  if( (task.flags&FLAG_LOG_TERSE) ||
      (task.flags&FLAG_LOG_FULL) ) 
    {
      report_set_filename();
      
      if( (task.flags&FLAG_LOGWRITER_MPIIO) )  {
	if( report_xml_mpiio(reportflags)!=IPM_OK ) 
	  {
	    IPMERR("Writing log using MPI-IO failed, trying serial\n");
	    report_xml_atroot(reportflags);
	  }
      }
      else {
	report_xml_atroot(reportflags);
      }
    }
#else /* HAVE_MPI */
  report_xml_local(reportflags);
#endif /* HAVE_MPI */
}
