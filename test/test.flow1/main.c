
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#include "../include/ipm_time.h"

#define INNER 10
#define OUTER 50

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char buf[256];

  MPI_Init( &argc, &argv);

  PMPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  PMPI_Comm_size( MPI_COMM_WORLD, &nprocs );

  MPI_Bcast(buf, 1, MPI_BYTE, 0, MPI_COMM_WORLD);

  for( i=0; i<OUTER; i++ ) {
    for( j=0; j<INNER; j++ ) {
      MPI_Bcast(buf, 2, MPI_BYTE, 0, MPI_COMM_WORLD);
      MPI_Bcast(buf, 3, MPI_BYTE, 0, MPI_COMM_WORLD);
    }

    for( j=0; j<INNER; j++ ) {
      MPI_Bcast(buf, 4, MPI_BYTE, 0, MPI_COMM_WORLD);
      MPI_Bcast(buf, 5, MPI_BYTE, 0, MPI_COMM_WORLD);
    }

    if( i==OUTER-1 ) {
      MPI_Bcast(buf, 7, MPI_BYTE, 0, MPI_COMM_WORLD);
    } 
    
    MPI_Bcast(buf, 6, MPI_BYTE, 0, MPI_COMM_WORLD);
    
    MPI_Bcast(buf, 8, MPI_BYTE, 0, MPI_COMM_WORLD);
  }

  MPI_Bcast(buf, 9, MPI_BYTE, 0, MPI_COMM_WORLD);

 
  MPI_Finalize();
}
