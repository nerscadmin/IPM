
/* ---- wrapping FFNAME ---- */
 
/*
 *
 */

FRET FFNAME(FPARAMS)
{
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

#if HAVE_COLLECTIVE   /* HAVE _COLLECTIVE */
#ifdef MPICH2
  extern void* MPIR_F_MPI_IN_PLACE;
  if (sbuf == MPIR_F_MPI_IN_PLACE) sbuf = MPI_IN_PLACE;
#endif
#if defined(OPEN_MPI) && OMPI_MAJOR_VERSION == 1 && OMPI_MINOR_VERSION <= 6
#include "openmpi/opal_config.h"
#include "openmpi/ompi/mpi/f77/constants.h"
  sbuf = (char *) OMPI_F2C_IN_PLACE(sbuf);
#endif
#endif

  *info=CFNAME(F2CARGS);
  
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

}


