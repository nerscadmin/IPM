
/* ---- wrapping __FFNAME__ ---- */
 
/*
 *
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
  if (*info==MPI_SUCCESS) 
    MPI_Status_c2f(&cstat, status);
#endif

#if HAVE_CREQ    /* HAVE_CREQ */ 
  if( *info==MPI_SUCCESS )
    *req=MPI_Request_c2f(creq);
#endif

#if HAVE_CCOMM_OUT /* HAVE _CCOMM_OUT */
  if( *info==MPI_SUCCESS ) 
    *comm_out=MPI_Comm_c2f(ccomm_out);
#endif

#if HAVE_CCOMM_INOUT /* HAVE _CCOMM_INOUT */
  if( *info==MPI_SUCCESS ) 
    *comm_inout=MPI_Comm_c2f(ccomm_inout);
#endif

#if HAVE_CGROUP_OUT /* HAVE _CGROUP_OUT */
  if( *info==MPI_SUCCESS )
    *group_out=MPI_Group_c2f(cgroup_out);
#endif
  IPM___CFNAME__(__F2CARGS__, tstart, tstop);

}


