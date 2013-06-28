
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define ROOT      0
#define REPEAT    5

int main( int argc, char* argv[] )
{
  int i;
  int myrank, nprocs;
  char *buf;
  int dsize;
  MPI_Status status;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  buf=(char*)malloc(SIZE*dsize);
  for( i=0; i<REPEAT; i++ )
    {
      if( (myrank%2) ) {
        MPI_Send( buf, SIZE, MPI_DOUBLE, myrank-1, 33, MPI_COMM_WORLD);
      }
      else {
        MPI_Recv( buf, SIZE, MPI_DOUBLE, myrank+1, 33, MPI_COMM_WORLD, &status);
      }
      
      MPI_Barrier(MPI_COMM_WORLD);
    }

  MPI_Finalize();
}
