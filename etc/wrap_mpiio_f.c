
/* ---- wrapping FFNAME ---- */
 
/*
 *
 */

#if 0

FRET FFNAME(FPARAMS)
{
#if HAVE_CREQ    /* HAVE _CREQ */ 
  MPI_Request creq; 
#endif
#if HAVE_CSTAT   /* HAVE _CSTAT */ 
  MPI_Status  cstat; 
#endif

  *info=CFNAME(F2CARGS);
  
#if HAVE_CSTAT   /* HAVE _CSTAT */ 
  if (*info==MPI_SUCCESS) 
    MPI_Status_c2f(&cstat, status);
#endif

#if HAVE_CREQ    /* HAVE _CREQ */ 
  if( *info==MPI_SUCCESS )
    *req=MPI_Request_c2f(creq);
#endif
}


#endif 
