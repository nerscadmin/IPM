
#include <mpi.h>

#include "ipm.h"
#include "config.h"
#include "hashtable.h"
#include "hashkey.h"
#include "GEN.calltable_mpi.h"
#include "GEN.fproto.mpi.h"

#ifdef HAVE_KEYHIST
#include "mod_keyhist.h"
#endif

int MPI_Finalize()
{
  int rv;
  unsigned idx, idx2;
  unsigned csite;
  IPM_KEY_TYPE key;

  /* --- monitoring action for MPI_Finalize --- */

 if(ipm_state==STATE_FINALIZED) { /* don't ipm_finalize twice */
  PMPI_Finalize();
 }

#ifdef HAVE_CALLPATH
  csite=get_callsite_id();
#else
  csite=0;
#endif 

  IPM_MPI_KEY(key, MPI_FINALIZE_ID_GLOBAL, 0, 0, 1, csite);
  IPM_HASH_HKEY(ipm_htable,key,idx);

#ifdef HAVE_MPI_TRACE
#ifdef HAVE_KEYHIST
  KEYHIST_TRACE(task.tracefile,key);
#else
  if( task.tracefile && task.tracestate) {
    fprintf(task.tracefile, "%s %d %d %d %d\n",
	    "MPI_Finalize", 0, 0, 0, csite);
  }
#endif
#endif


#ifdef HAVE_KEYHIST
  IPM_XHASH_HKEY(ipm_xhtable,last_hkey,key,idx2);
  ipm_xhtable[idx2].t_tot+=(ipm_wtime()-last_tstamp);
  ipm_xhtable[idx2].count++;
  KEY_ASSIGN(last_hkey,key);
#endif

  ipm_htable[idx].count++;
  ipm_htable[idx].t_min=0.0;
  ipm_htable[idx].t_max=0.0;
  ipm_htable[idx].t_tot=0.0;


#ifdef HAVE_POSIXIO
  modules[IPM_MODULE_POSIXIO].state=STATE_NOTACTIVE;
#endif 
  
#ifndef DELAYED_MPI_FINALIZE
  ipm_finalize(0);
  rv = PMPI_Finalize();
  return rv;
#endif /* DELAYED_MPI_FINALIZE */
  
  return MPI_SUCCESS;
}


void MPI_FINALIZE_F(int *ierr)
{
  *ierr = MPI_Finalize();
}

