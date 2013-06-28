
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define REPEAT    5

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int dsize;

  MPI_Init( &argc, &argv );
  
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Type_size(DATATYPE, &dsize);

  sbuf=(char*)malloc(SIZE*dsize*nprocs);
  rbuf=(char*)malloc(SIZE*dsize*nprocs);

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Alltoall( sbuf, SIZE, DATATYPE,
		    rbuf, SIZE, DATATYPE,
		    MPI_COMM_WORLD );
    }

  MPI_Finalize();
}
