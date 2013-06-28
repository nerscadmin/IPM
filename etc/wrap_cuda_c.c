
/** HEADER_BEGIN **/

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "ipm_core.h"
#include "ipm_time.h"
#include "hashtable.h"
#include "perfdata.h"
#include "mod_cuda.h"

#include "vector_types.h"
#include "cuda_runtime_api.h"
#include "cuda.h"

#ifdef HAVE_DYNLOAD
#include <execinfo.h>
#include <dlfcn.h>
#endif /* HAVE_DYNLOAD */

#ifdef HAVE_KEYHIST
#include "mod_keyhist.h"
#endif

/* #define HAVE_CUDA_TRACE 1 */
#define HAVE_CUDA_KERNELTIMING 1
#define HAVE_CUDA_HOST_IDLE    1

#include <GEN.calltable_cuda.h>

cudaStream_t curr_stream_rt=0;

/** HEADER_END **/


__CRET__ __real___CFNAME__(__CPARAMS__);


/* ---- wrapping __CFNAME__ ---- */
/*
 * strings in the form __IDENT__ are replaced by the wrapper script
 *
 * CRET         : __CRET__
 * CFNAME       : __CFNAME__ 
 * CPARAMS      : __CPARAMS__
 * CARGS        : __CARGS__
 * CARGFMT      : __CARGFMT__
 * CRETFMT      : __CRETFMT__
 * GET_SSIZE    : __GET_SSIZE__
 * GET_RSIZE    : __GET_RSIZE__
 * GET_RANK     : __GET_RANK__
 * GET_BYTES    : __GET_BYTES__
 * RETURN_VALUE : __RETURN_VALUE__
 */

#ifdef HAVE_DYNLOAD
__CRET__ __CFNAME__(__CPARAMS__)
#else
__CRET__ __wrap___CFNAME__(__CPARAMS__)
#endif
{
  static int loaded=0;
  static __CRET__ (*__CFNAME___real)(__CPARAMS__);
  int i, slot;
  int ibytes, irank;
  int csite, idx, idx2, regid; 
  double tstart, tstop, t;
  IPM_KEY_TYPE key;
  int oldstate;

#if __RETURN_VALUE__
  __CRET__ rv;
#endif
  
#ifdef HAVE_DYNLOAD
  if(!loaded) {
    __CFNAME___real=0;
    __CFNAME___real=(__CRET__ (*)(__CPARAMS__)) dlsym(RTLD_NEXT, 
						      "__CFNAME__");
    
    if(!dlerror()) loaded=1;
    else {
      fprintf(stderr, "Error loading __CFNAME__ \n");
      /* handle error */
    }
  }
#endif /* HAVE_DYNLOAD */

  if( ipm_state==STATE_NOTINIT ) {
#ifndef HAVE_MPI
    ipm_init(0);
#endif
  }

  /* --- if not active, just call original function and return --- */
  if( ipm_state!=STATE_ACTIVE )  { 
#if __RETURN_VALUE__
#ifdef HAVE_DYNLOAD
    rv=__CFNAME___real(__CARGS__);
#else
    rv=__real___CFNAME__(__CARGS__); 
#endif
    return rv;
#else
#ifdef HAVE_DYNLOAD
    __CFNAME___real(__CARGS__);
#else
    __real___CFNAME__(__CARGS__); 
#endif
    return;
#endif
  }
  

  /* remember old state to restore to later */
  oldstate=ipm_state;
  ipm_state=STATE_NOTACTIVE;
  
#ifdef HAVE_CUDA_KERNELTIMING
#if (__CFID__ == CUDA_CONFIGURECALL_ID)
  curr_stream_rt = stream;
#endif 
#if (__CFID__ == CUDA_LAUNCH_ID)
  IPM_CUDA_RT_LAUNCH_PRE(entry, curr_stream_rt);
#endif 
#if (__CFID__ == CUDA_CULAUNCHGRID_ID || __CFID__ == CUDA_CULAUNCH_ID )
  IPM_CUDA_DR_LAUNCH_PRE(f, 0);
#endif 
#if (__CFID__ == CUDA_CULAUNCHGRIDASYNC_ID)
  IPM_CUDA_DR_LAUNCH_PRE(f, hStream);
#endif 
#endif /* HAVE_CUDA_KERNELTIMING */
  
#ifdef HAVE_CUDA_HOST_IDLE
  
  /* 1D and 2D memcpy variants */
#if (__CFID__ == CUDA_MEMCPY_ID) || (__CFID__ == CUDA_MEMCPY2D_ID) ||	\
  (__CFID__ == CUDA_MEMCPYTOARRAY_ID) || (__CFID__ == CUDA_MEMCPYFROMARRAY_ID) || \
  (__CFID__ == CUDA_MEMCPYTOSYMBOL_ID) || (__CFID__ == CUDA_MEMCPYFROMSYMBOL_ID) || \
  (__CFID__ == CUDA_MEMCPYARRAYTOARRAY_ID)  || (__CFID__ == CUDA_MEMCPY2DARRAYTOARRAY_ID) || \
  (__CFID__ == CUDA_MEMCPY2DTOARRAY_ID) || (__CFID__ == CUDA_MEMCPY2DFROMARRAY_ID)
  if( kind==cudaMemcpyDeviceToHost ) {
    IPM_CUDA_HOST_IDLE(0);
  }
#endif 

  /* 3d memcpy */
#if (__CFID__ == CUDA_MEMCPY3D_ID)
  if( p && (p->kind)==cudaMemcpyDeviceToHost )  {
    IPM_CUDA_HOST_IDLE(0);
  }

#if (__CFID__ == CUDA_CUMEMCPYDTOH_ID )
  IPM_CUDA_HOST_IDLE(0);
#endif

#endif
  
  /* free and freearray */
#if (__CFID__ == CUDA_FREE_ID) || (__CFID__ == CUDA_FREEARRAY_ID)
  IPM_CUDA_HOST_IDLE(0);
#endif 
  
#endif /* HAVE_CUDA_HOST_IDLE */
  
  IPM_TIMESTAMP(tstart);

  /* ---- invoke the wrapped function ---- */
#if __RETURN_VALUE__
#ifdef HAVE_DYNLOAD
  rv=__CFNAME___real(__CARGS__);
#else
  rv=__real___CFNAME__(__CARGS__); 
#endif
#else
#ifdef HAVE_DYNLOAD
  __CFNAME___real(__CARGS__);
#else
  __real___CFNAME__(__CARGS__); 
#endif
#endif 
  
  
  IPM_TIMESTAMP(tstop); 
  t=tstop-tstart;

#ifdef HAVE_CUDA_TRACE
  fprintf(stderr, "%.5e    %.5e    %s\n", 
	  tstart-IPM_TIMEVAL(task.t_start), 
	  t, "__CFNAME__");
#endif 
  
#ifdef HAVE_CUDA_KERNELTIMING

#if (__CFID__ == CUDA_LAUNCH_ID)
  IPM_CUDA_RT_LAUNCH_POST(curr_stream_rt);
#endif
#if (__CFID__ == CUDA_CULAUNCHGRID_ID)
  IPM_CUDA_DR_LAUNCH_POST(0);
#endif 
#if (__CFID__ == CUDA_CULAUNCHGRIDASYNC_ID)
  IPM_CUDA_DR_LAUNCH_POST(hStream);
#endif 
  
#if (__CFID__ == CUDA_EVENTSYNCHRONIZE_ID ) ||  \
  (__CFID__ == CUDA_MEMCPY_ID) ||		\
  (__CFID__ == CUDA_FREE_ID) ||			\
  (__CFID__ == CUDA_CONFIGURECALL_ID)
  ipm_cuda_kerneltiming();  
#endif

#if (__CFID__ == CUDA_CUMEMCPYDTOH_ID )   
  ipm_cuda_kerneltiming();  
#endif

#if (__CFID__ == CUDA_CUMODULEGETFUNCTION_ID ) 
  if( hfunc ) {
    IPM_CUDA_ADDPTR(((void*)*hfunc), ((char*)name) );
  }
#endif
  
#endif /* HAVE_CUDA_KERNELTIMING */
  
  ipm_state=oldstate;
  
  /* determine hash key parameters */
  __GET_BYTES__(ibytes);
  regid=ipm_rstackptr->id;
  csite=0;
  
  /* build the key */
  IPM_CUDA_KEY(key, __CFID___GLOBAL, 0, ibytes, regid, csite);

  /* update htable */
  IPM_HASH_HKEY(ipm_htable, key, idx);
  IPM_HASHTABLE_ADD(idx,t);

#ifdef HAVE_KEYHIST
  IPM_XHASH_HKEY(ipm_xhtable,last_hkey,key,idx2);
  ipm_xhtable[idx2].t_tot+=(tstart-last_tstamp);
  ipm_xhtable[idx2].count++;
  KEY_ASSIGN(last_hkey,key);
  last_tstamp=tstop;
#endif
  
#if __RETURN_VALUE__
    return rv;
#else
    return;
#endif
}

