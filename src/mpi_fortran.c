#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#include "ipm.h"
#include "ipm_core.h"
#include "mod_mpi.h"
#include "GEN.calltable_mpi.h"
#include "GEN.fproto.mpi.h"



static MPI_Request* alloc_requests(int count) 
{
  static MPI_Request *req_array = 0;
  static int req_array_size = 0;
  
  if (req_array_size == 0) 
    {
      req_array = (MPI_Request*) IPM_MALLOC(count * sizeof(MPI_Request));
      req_array_size = count;
    }
  else if (count > req_array_size) 
    {
      req_array = (MPI_Request*) IPM_REALLOC(req_array, count * sizeof(MPI_Request));
      req_array_size = count;
    }

  return req_array;
}


static MPI_Status* alloc_statuses(int count) 
{
  static MPI_Status *stat_array = 0;
  static int stat_array_size = 0;

  if (stat_array_size == 0) 
    {
      /* -- never used: initialize -- */
      stat_array = (MPI_Status*) IPM_MALLOC(count * sizeof(MPI_Status));
      stat_array_size = count;
    } 
  else if (count > stat_array_size) 
    {
      /* -- not enough room: expand -- */
      stat_array = (MPI_Status*) IPM_REALLOC(stat_array, count * sizeof(MPI_Status));
      stat_array_size = count;
    }
  return stat_array;
}



void MPI_TEST_F(MPI_Fint *request, MPI_Fint *flag,
		MPI_Fint *status, MPI_Fint *info)
{
  MPI_Request creq;
  MPI_Status  cstat;

  creq = MPI_Request_f2c(*request);
  *info = MPI_Test(&creq, flag, &cstat);
  if( *info != MPI_SUCCESS ) return;
			       
  *request = MPI_Request_c2f(creq);
  
  if(status!=0 && flag) 
    MPI_Status_c2f(&cstat, status);
}

void MPI_WAIT_F(MPI_Fint *request, 
		MPI_Fint *status, MPI_Fint *info)
{
  MPI_Request creq;
  MPI_Status  cstat;

  creq = MPI_Request_f2c(*request);
  *info = MPI_Wait(&creq, &cstat);
  *request = MPI_Request_c2f(creq);

  if(status!=0 && *info==MPI_SUCCESS )
    MPI_Status_c2f(&cstat, status);
}

void MPI_TESTANY_F(MPI_Fint *num, MPI_Fint *requests, 
		   MPI_Fint *indx, MPI_Fint *flag, 
		   MPI_Fint *status, MPI_Fint *info)
{
  int i;
  MPI_Request *creq;
  MPI_Status cstat;

  if(*num > 0 ) {
    creq = alloc_requests(*num);

    for( i=0; i<*num; i++ ) {
      creq[i] = MPI_Request_f2c(requests[i]);
    }
  }

  *info = MPI_Testany(*num, creq, indx, flag, &cstat);

  if( *info==MPI_SUCCESS )
    {
      if( *flag && *indx>=0 ) {
	requests[*indx] = MPI_Request_c2f(creq[*indx]);
	(*indx)++;
      }
      if( status!=0 ) {
	MPI_Status_c2f(&cstat, status);
      }
    }
}

void MPI_WAITANY_F(MPI_Fint *num, 
		   MPI_Fint *requests, MPI_Fint *indx, 
		   MPI_Fint *status, MPI_Fint *info)
{
  int i;
  MPI_Request *creq;
  MPI_Status cstat;
  
  if(*num > 0 ) {
    creq = alloc_requests(*num);

    for( i=0; i<*num; i++ ) {
      creq[i] = MPI_Request_f2c(requests[i]);
    }
  }

  *info = MPI_Waitany(*num, creq, indx, &cstat);

  if( *info==MPI_SUCCESS )
    {
      if( *indx>=0 ) {
	requests[*indx] = MPI_Request_c2f(creq[*indx]);
	(*indx)++;
      }
      if( status!=0 ) {
	MPI_Status_c2f(&cstat, status);
      }
    }
}


void MPI_TESTALL_F(MPI_Fint *num, MPI_Fint *requests, 
		   MPI_Fint *flag, MPI_Fint *statuses, 
		   MPI_Fint *info)
{
  int i;
  MPI_Request *creq;
  MPI_Status *cstat;

  if(*num > 0 ) {
    creq = alloc_requests(*num);
    cstat = alloc_statuses(*num);

    for( i=0; i<*num; i++ ) {
      creq[i] = MPI_Request_f2c(requests[i]);
    }
  }

  *info = MPI_Testall(*num, creq, flag, cstat);

  for( i=0; i<*num; i++ ) {
    requests[i] = MPI_Request_c2f(creq[i]);
  }
  if( statuses!=0 && *info==MPI_SUCCESS && *flag ) {
    for( i=0; i<*num; i++ ) {
      MPI_Status_c2f( &(cstat[i]), &(statuses[i]));
    } 
  }  
}

void MPI_WAITALL_F(MPI_Fint *num, 
		   MPI_Fint *requests, MPI_Fint *statuses, 
		   MPI_Fint *info)
{
  int i;
  MPI_Request *creq;
  MPI_Status *cstat;

  if(*num > 0 ) {
    creq = alloc_requests(*num);
    cstat = alloc_statuses(*num);

    for( i=0; i<*num; i++ ) {
      creq[i] = MPI_Request_f2c(requests[i]);
    }
  }

  *info = MPI_Waitall(*num, creq, cstat);
  
  for( i=0; i<*num; i++ ) {
    requests[i] = MPI_Request_c2f(creq[i]);
  }
  if( statuses!=0 && *info==MPI_SUCCESS ) {
    for( i=0; i<*num; i++ ) {
      MPI_Status_c2f( &(cstat[i]), &(statuses[i]));
    } 
  }  
}


void MPI_TESTSOME_F(MPI_Fint *inum, MPI_Fint *requests, 
		    MPI_Fint *onum, MPI_Fint *indices, 
		    MPI_Fint *statuses, MPI_Fint *info)
{
  int i, j, found;
  MPI_Request *creq;
  MPI_Status *cstat;

  if(*inum > 0 ) {
    creq = alloc_requests(*inum);
    cstat = alloc_statuses(*inum);

    for( i=0; i<*inum; i++ ) {
      creq[i] = MPI_Request_f2c(requests[i]);
    }
  }
  
  *info = MPI_Testsome(*inum, creq, onum, indices, cstat);
  
  if( *info==MPI_SUCCESS )
    {
      for (i=0; i<*inum; i++) {
	if (i < *onum) {
	  requests[indices[i]] = MPI_Request_c2f(creq[indices[i]]);
	} else {
	  found = j = 0;
	  while ( (!found) && (j<*onum) ) {
	    if (indices[j++] == i) found = 1;
	  }
	  if (!found) requests[i] = MPI_Request_c2f(creq[i]);
	}
      }

      if( statuses!=0 ) {
	for (i=0; i<*onum; i++) {
	  MPI_Status_c2f(&cstat[i], &(statuses[i]));
	  /* See the description of testsome in the standard;
	     the Fortran index ranges are from 1, not zero */
	  if (indices[i] >= 0) indices[i]++;
	}
      }
    }
}


void MPI_WAITSOME_F(MPI_Fint *inum, MPI_Fint *requests, 
		    MPI_Fint *onum, MPI_Fint *indices, 
		    MPI_Fint *statuses, MPI_Fint *info)
{
  int i, j, found;
  MPI_Request *creq;
  MPI_Status *cstat;

  if(*inum > 0 ) {
    creq = alloc_requests(*inum);
    cstat = alloc_statuses(*inum);

    for( i=0; i<*inum; i++ ) {
      creq[i] = MPI_Request_f2c(requests[i]);
    }
  }
  
  *info = MPI_Waitsome(*inum, creq, onum, indices, cstat);

  if( *info==MPI_SUCCESS )
    {
      for (i=0; i<*inum; i++) {
	if (i < *onum) {
	  requests[indices[i]] = MPI_Request_c2f(creq[indices[i]]);
	} else {
	  found = j = 0;
	  while ( (!found) && (j<*onum) ) {
	    if (indices[j++] == i) found = 1;
	  }
	  if (!found) requests[i] = MPI_Request_c2f(creq[i]);
	}
      }

      if( statuses!=0 ) {
	for (i=0; i<*onum; i++) {
	  MPI_Status_c2f(&cstat[i], &(statuses[i]));
	  /* See the description of testsome in the standard;
	     the Fortran index ranges are from 1, not zero */
	  if (indices[i] >= 0) indices[i]++;
	}
      }
    }
}
