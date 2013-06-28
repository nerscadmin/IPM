
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#include "../include/ipm_time.h"


#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define ROOT      0
#define REPEAT    1

int main( int argc, char* argv[] )
{
  int i;
  int myrank, nprocs;
  char *buf;
  int dsize;
  double t1, t2;
  
  int req, prov;
  req = MPI_THREAD_SINGLE;

  MPI_Init( &argc, &argv);

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);
  
  buf=(char*)malloc(SIZE*dsize);

  IPM_TIMESTAMP(t1);
  for( i=0; i<REPEAT; i++ )
    {
      MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    }
  IPM_TIMESTAMP(t2);

  fprintf(stderr, "Processed %d events in %f seconds with IPM\n", 
	  REPEAT, (t2-t1));

  


  IPM_TIMESTAMP(t1);
  for( i=0; i<REPEAT; i++ )
    {
      PMPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    }
  IPM_TIMESTAMP(t2);

  fprintf(stderr, "Processed %d events in %f seconds without IPM\n", 
	  REPEAT, (t2-t1));

  MPI_Finalize();

  fopen("/dev/null", "r");  
}
