
/** HEADER_BEGIN **/

#include <mpi.h>

#include "ipm.h"
#include "ipm_core.h"

#ifndef MPI3CONST
#if MPI_VERSION >= 3 
#define MPI3CONST const
#else
#define MPI3CONST 
#endif 
#endif


/** HEADER_END **/


/* ---- wrapping __FFNAME__ ---- */
/*
 * strings in the form __IDENTIFIER__ are replaced
 * by the wrapper script
 *
 * CFNAME     __CFNAME__
 * FFNAME     __FFNAME__
 * CPARAMS    __CPARAMS__
 * FPARAMS    __FPARAMS__
 * F2CARGS    __F2CARGS__
 * FRET       __FRET__
 */

extern void IPM___CFNAME__(__CPARAMS__, double tstart, double tstop);

extern void p__FFNAME__(__FPARAMS__);

__FRET__ __FFNAME__(__FPARAMS__)
{
  double tstart, tstop;

#if HAVE_CREQ    /* HAVE _CREQ */ 
  MPI_Request creq; 
#endif
#if HAVE_CSTAT   /* HAVE _CSTAT */ 
  MPI_Status  cstat; 
#endif
#if HAVE_CCOMM_OUT
  MPI_Comm ccomm_out;
#endif           /* HAVE _CCOMM_OUT */
#if HAVE_CCOMM_INOUT
  MPI_Comm ccomm_inout;
#endif           /* HAVE _CCOMM_INOUT */

#if HAVE_CCOMM_INOUT
  ccomm_inout = MPI_Comm_f2c(*comm_inout);
#endif 

#if HAVE_CGROUP_OUT /* HAVE _CGROUP_OUT */
  MPI_Group cgroup_out;
#endif

  ipm_in_fortran_pmpi = IPM_IN_FORTRAN_PMPI;

  IPM_TIMESTAMP(tstart);
  p__FFNAME__(__FARGS__);
  IPM_TIMESTAMP(tstop);

  ipm_in_fortran_pmpi = IPM_NOT_IN_FORTRAN_PMPI;

  if( ipm_state!=STATE_ACTIVE ) {
    return;
  }
  
#if HAVE_CSTAT   /* HAVE_CSTAT */ 
  if ( *info==MPI_SUCCESS )
    MPI_Status_f2c(status, &cstat);
#endif

#if HAVE_CREQ    /* HAVE_CREQ */ 
  if( *info==MPI_SUCCESS )
    creq=MPI_Request_f2c(*req);
#endif

#if HAVE_CCOMM_OUT /* HAVE _CCOMM_OUT */
  if( *info==MPI_SUCCESS ) 
    ccomm_out=MPI_Comm_f2c(*comm_out);
#endif

#if HAVE_CCOMM_INOUT /* HAVE _CCOMM_INOUT */
  if( *info==MPI_SUCCESS ) 
    ccomm_inout=MPI_Comm_f2c(*comm_inout);
#endif

#if HAVE_CGROUP_OUT /* HAVE _CGROUP_OUT */
  if( *info==MPI_SUCCESS )
    cgroup_out=MPI_Group_f2c(*group_out);
#endif
  IPM___CFNAME__(__F2CARGS__, tstart, tstop);

}


