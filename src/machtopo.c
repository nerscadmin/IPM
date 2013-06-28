
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_sizes.h"

#ifdef HAVE_MPI

void ipm_get_machtopo()
{
  unsigned i, j, np;
  char *allnames;
  char *unique;
  unsigned nunique;
  int x;

  np = task.ntasks;

  if( task.taskid==0 ) {
    allnames = IPM_CALLOC( np, MAXSIZE_HOSTNAME );
    unique   = IPM_CALLOC( np, MAXSIZE_HOSTNAME );

    if( !allnames || !unique ) {
      IPMERR("Out of memory allocating buffers in ipm_get_machtopo\n");
      return;
    }
  }
  
  IPM_GATHER( task.hostname, MAXSIZE_HOSTNAME, MPI_BYTE, 
	      allnames, MAXSIZE_HOSTNAME, MPI_BYTE, 
	      0, MPI_COMM_WORLD );

  nunique=0;
  if( task.taskid==0 ) 
    {
      for( i=0; i<np; i++ ) {
	for( j=0; j<=i; j++ ) {
	  if( !(unique[j*MAXSIZE_HOSTNAME]) ) {
	    strncpy(&(unique[j*MAXSIZE_HOSTNAME]), 
		    &(allnames[i*MAXSIZE_HOSTNAME]), 
		    MAXSIZE_HOSTNAME);
	    nunique++;
	    break;
	  }
	  if( !strncmp(&(unique[j*MAXSIZE_HOSTNAME]), 
		       &(allnames[i*MAXSIZE_HOSTNAME]),
		       MAXSIZE_HOSTNAME) )
	    break;
	}
      }

      task.nhosts=nunique;
    }

  IPM_BCAST( &(task.nhosts), 1, MPI_INT, 0, MPI_COMM_WORLD );

  if( task.taskid==0 ) {
    if( allnames ) IPM_FREE(allnames); 
    if( unique ) IPM_FREE(unique);
  }
    
}

#endif
