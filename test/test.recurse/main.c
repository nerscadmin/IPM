
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#include "../include/ipm_time.h"

void fibsender(int myrank, int n);
void fibreceiver(int myrank, int n);


#define REPEAT    1

int main( int argc, char* argv[] )
{
  int myrank, nprocs;

  
  MPI_Init( &argc, &argv);

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );

  if( nprocs%2 ) {
    fprintf(stderr, "Use even number of procs!\n");
    exit(1);
  }
  
  fibsender(myrank, 5);
  fibreceiver(myrank, 5);
  
  MPI_Finalize();
}


void fibsender(int myrank, int n) {
  char buf[1];
  
  if( n==0 ) return;

  if( n==1 ) {
    MPI_Send( &buf, 1, MPI_BYTE, myrank+1, 33, MPI_COMM_WORLD );
  }
  else {
    fibsender(myrank, n-1);
    fibsender(myrank, n-2);
  }
}

void fibreceiver(int myrank, int n) {
  char buf[1];
  MPI_Status stat;
  
  if( n==0 ) return;

  if( n==1 ) {
    MPI_Recv( &buf, 1, MPI_BYTE, myrank-1, 33, MPI_COMM_WORLD, &stat );
  }
  else {
    fibsender(myrank, n-1);
    fibsender(myrank, n-2);
  }
}
