
#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  int myrank, nprocs; 

  MPI_Init( &argc, &argv );

  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

  fprintf(stderr, "Hello from rank %d of %d\n",
	  myrank, nprocs );

  MPI_Finalize();
}
