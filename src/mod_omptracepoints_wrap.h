
#include <stdio.h>
#include <omp.h>
#include "hashtable.h"

#undef TP
#define TP(func_) func_

#ifdef TRACEPOINTS_PGI
#undef TP
#define TP(func_) _mp_trace_##func_
#endif

#ifdef TRACEPOINTS_CRAY
#undef TP
#define TP(func_) __pat_tp_omp_##func_
#endif


#ifdef HAVE_OMP_TRACE
#define DO_OMP_TRACE(func_)					\
  if( task.tracefile ) {					\
    fprintf(task.tracefile, func_ " tid=%d wtime=%f\n",		\
	    omp_get_thread_num(), ipm_wtime());			\
  }								
#else 
#define DO_OMP_TRACE
#endif /* HAVE_OMP_TRACE */


#define OMP_UPDATE_HTABLE(actv_,reg_,csite_,rank_,tid_, time_)	\
  {								\
    int idx;							\
    IPM_KEY_TYPE key;						\
    								\
    /* (key_,actv_,reg_,csite_,rank_,tid_,bytes_) */	        \
    IPM_MAKE_KEY(key, actv_, reg_, csite_, rank_, tid_, 0);	\
    IPM_HASH_HKEY(ipm_htable, key, idx);			\
    ipm_htable[idx].count++;					\
    ipm_htable[idx].t_tot+=time_;				\
    if( time_>ipm_htable[idx].t_max ) ipm_htable[idx].t_max=time_; \
    if( time_<ipm_htable[idx].t_min ) ipm_htable[idx].t_min=time_; \
  }								\

int num_levels;
    
/* executed my master thread only */
void TP(parallel_enter)() {
  if( ipm_state==STATE_NOTINIT ) {
    ipm_init(0);
  }

  DO_OMP_TRACE("parallel_enter");

#pragma omp atomic
 num_levels++;
 if (num_levels > 1) return;

  ompstats[0].tenter = ipm_wtime();
}

/* executed by all threads */
void TP(parallel_begin)() {
  int tid;

  DO_OMP_TRACE("parallel_begin");

  if (num_levels != 1) return;

  tid = omp_get_thread_num();
  if( tid==0 ) {
    nthreads = omp_get_num_threads();
    if( nthreads>maxthreads ) maxthreads=nthreads;
  } else {
    ompstats[tid].tenter = ipm_wtime(); 
  }
  ompstats[tid].nenter++;
}

/* executed by all threads */
void TP(parallel_end)() {
  int tid;
  double tstamp;
  double time;

  DO_OMP_TRACE("parallel_end");

  if (num_levels != 1) return;

  tid = omp_get_thread_num();
  tstamp = ipm_wtime();
  ompstats[tid].twork  = (tstamp - ompstats[tid].tenter);
  ompstats[tid].tenter = tstamp;
}

/* executed by master thread only */
void TP(parallel_exit)() {
  int i, tid, regid;
  double tstamp, tpar;

  DO_OMP_TRACE("parallel_exit");
  
#pragma omp atomic
  num_levels--;
  if (num_levels != 0) return;

  tstamp = ipm_wtime();
  for( i=0; i<nthreads; i++ ) {
    ompstats[i].tidle = (tstamp - ompstats[i].tenter); 
  }
  tpar = ompstats[0].tidle + ompstats[0].twork;

  regid=ipm_rstackptr->id;

  /* actv_,reg_,csite_,rank_,tid_, time_ */
  OMP_UPDATE_HTABLE(OMP_PARALLEL_ID_GLOBAL, regid, 0, 0, 0, tpar);


  for( i=0; i<nthreads; i++ )
    {
      OMP_UPDATE_HTABLE(OMP_IDLE_ID_GLOBAL, regid, 0, 
			task.taskid, i, ompstats[i].tidle);
    }
}

void TP(master_enter)() {
  DO_OMP_TRACE("master_enter");
}
void TP(master_exit)() {
  DO_OMP_TRACE("master_exit");
}

void TP(single_enter)() {
  DO_OMP_TRACE("single_enter");
}
void TP(single_exit)() {
  DO_OMP_TRACE("single_exit");
}

void TP(loop_enter)() {
  DO_OMP_TRACE("loop_enter");
}
void TP(loop_exit)() {
  DO_OMP_TRACE("loop_exit");
}

void TP(sections_enter)() {
  DO_OMP_TRACE("sections_enter");
}
void TP(section_begin)() {
  DO_OMP_TRACE("section_begin");
}
void TP(section_end)() {
  DO_OMP_TRACE("section_end");
}
void TP(sections_exit)() {
  DO_OMP_TRACE("sections_exit");
}

void TP(workshare_enter)() {
  DO_OMP_TRACE("workshare_enter");
}
void TP(workshare_exit)() {
  DO_OMP_TRACE("workshare_exit");
}

void TP(task_enter)() {
  DO_OMP_TRACE("task_enter");
}
void TP(task_exit)() {
  DO_OMP_TRACE("task_exit");
}

void TP(task_begin)() {
  DO_OMP_TRACE("task_begin");
}
void TP(task_end)() {
  DO_OMP_TRACE("task_end");
}
