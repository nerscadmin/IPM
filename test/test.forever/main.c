
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define ROOT      0

int main( int argc, char* argv[] )
{
  int i;
  int myrank, nprocs;
  char *buf;
  int dsize;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  buf=(char*)malloc(SIZE*dsize);

  while(1) /* forever */
    {
      MPI_Bcast( buf, SIZE, DATATYPE, ROOT, MPI_COMM_WORLD );
    }

  MPI_Finalize();
}
