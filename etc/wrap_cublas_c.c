
/** HEADER_BEGIN **/

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ipm_core.h"
#include "ipm_time.h"
#include "hashtable.h"
#include "perfdata.h"
#include "mod_cublas.h"

#include "cuda.h"
#include "cublas.h"

#ifdef HAVE_DYNLOAD
#include <execinfo.h>
#include <dlfcn.h>
#endif /* HAVE_DYNLOAD */

#ifdef HAVE_KEYHIST
#include "mod_keyhist.h"
#endif

#include <GEN.calltable_cublas.h>

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
#if __RETURN_VALUE__
  __CRET__ rv;
#endif

  double tstart, tstop, t;
  int oldstate;
  int idx, regid;
  int ibytes;
  IPM_KEY_TYPE key;
  
  
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
    ipm_init(0);
  }

  IPM_TIMESTAMP(tstart);

#if 0 /*(__CFID__ == CUBLAS_ZGEMM_ID ) */
#else  
  oldstate=ipm_state;
  ipm_state=STATE_NOTACTIVE;
#endif

  /* invoke wrapped function */
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

#if 0 /* (__CFID__ == CUBLAS_ZGEMM_ID ) */
#else  
  ipm_state=oldstate;
#endif

  if( ipm_state!=STATE_ACTIVE ) { 
#if __RETURN_VALUE__
    return rv;
#else
    return;
#endif
  }

  IPM_TIMESTAMP(tstop); 
  t=tstop-tstart;
    
  regid=ipm_rstackptr->id;
  __GET_BYTES__(ibytes);


  /* build the key */
  IPM_CUFFT_KEY(key, __CFID___GLOBAL, 0, ibytes, regid, 0);

  /* update htable */
  IPM_HASH_HKEY(ipm_htable, key, idx);
  IPM_HASHTABLE_ADD(idx,t);
  
#if __RETURN_VALUE__
    return rv;
#else
    return;
#endif
}

