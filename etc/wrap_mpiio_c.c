
/** HEADER_BEGIN **/

#include <mpi.h>
/* #include <mpio.h> */

#include "ipm.h"
#include "ipm_core.h"
#include "hashtable.h"
#include "mod_mpi.h"
#include "mod_mpiio.h"
#include "GEN.calltable_mpi.h"
#include "GEN.calltable_mpiio.h"

#ifdef HAVE_CALLPATH
#include "mod_callpath.h"
#endif

#ifdef HAVE_KEYHIST
#include "mod_keyhist.h"
#endif


#include "regstack.h"

/** HEADER_END **/


/* ---- wrapping __CFNAME__ ---- */
/*
 * strings in the form __IDENT__ are replaced by the wrapper script
 *
 * CRET       __CRET__
 * CFNAME     __CFNAME__ 
 * CPARAMS    __CPARAMS__
 * CARGS      __CARGS__
 * CFMT       __CFMT__
 * GET_SSIZE  __GET_SSIZE__
 * GET_RSIZE  __GET_RSIZE__
 * GET_RANK   __GET_RANK__
 * GET_BYTES  __GET_BYTES__
 */

__CRET__ __CFNAME__(__CPARAMS__)
{
  __CRET__ rv;
  int bytes, irank;
  double tstart, tstop, t;
  IPM_KEY_TYPE key;
  int csite, idx, idx2;
  int regid;

  if( ipm_state==STATE_NOTINIT ) {
    ipm_init(0);
  }
  
  IPM_TIMESTAMP(tstart);
  rv = __PCFNAME__(__CARGS__);
  IPM_TIMESTAMP(tstop);

  if( ipm_state!=STATE_ACTIVE ) {
    return rv;
  }

  t=tstop-tstart;

  bytes=0; irank=0;
  __GET_BYTES__(bytes);
  __GET_RANK__(irank);

  if( irank==MPI_PROC_NULL || irank==MPI_ANY_SOURCE )
    irank = IPM_RANK_NULL; 

#ifdef HAVE_CALLPATH
  csite=get_callsite_id();
#else
  csite=0;
#endif 

  regid=ipm_rstackptr->id;

#ifdef HAVE_MPI_TRACE
  if( task.tracefile && task.tracestate) {
    fprintf(task.tracefile, "%s %d %d %d %d\n",
	    "__CFNAME__", irank, bytes, regid, csite);
  }
#endif


  IPM_MPI_KEY(key, __CFID___GLOBAL, irank, bytes, 
	      regid, csite);

  IPM_HASH_HKEY(ipm_htable, key, idx);

#ifdef HAVE_KEYHIST
  IPM_XHASH_HKEY(ipm_xhtable,last_hkey,key,idx2);
  ipm_xhtable[idx2].t_tot+=(tstart-last_tstamp);
  ipm_xhtable[idx2].count++;
  KEY_ASSIGN(last_hkey,key);
  last_tstamp=tstop;
#endif

  IPM_HASHTABLE_ADD(idx,t);

  return rv;
}

