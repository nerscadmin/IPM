
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define REPEAT 10

int main( int argc, char* argv[] )
{
  int i, res;
  int myrank, nprocs;
  MPI_Group g1;
  MPI_Comm com1, com2, com3;
  
  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );

  for( i=0; i<REPEAT; i++ ) {
    MPI_Comm_group(MPI_COMM_WORLD, &g1);
    MPI_Comm_create(MPI_COMM_WORLD, g1, &com1);
    
    MPI_Comm_compare(MPI_COMM_WORLD, com1, &res);
    MPI_Comm_dup(MPI_COMM_WORLD, &com2);
    
    MPI_Comm_split(MPI_COMM_WORLD, myrank, myrank, &com3);

    MPI_Comm_free(&com2);
    MPI_Comm_free(&com3);

  }


  MPI_Finalize();
  return 0;
}
