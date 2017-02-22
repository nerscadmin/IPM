
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define REPEAT    10

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int dsize;
  MPI_Comm newcomm;
  MPI_Request request;
  int newrank, newnprocs;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Type_size(DATATYPE, &dsize);

  sbuf=(char*)malloc(SIZE*dsize);
  rbuf=(char*)malloc(SIZE*dsize);

  /* Create a new communicator for odd and even ranks */
  MPI_Comm_split( MPI_COMM_WORLD, myrank%2, myrank, &newcomm );
  MPI_Comm_rank( newcomm, &newrank );
  MPI_Comm_size( newcomm, &newnprocs );

/*
  fprintf(stderr, "Old: %d/%d, new: %d/%d\n", myrank, nprocs,
  newrank, newnprocs);
*/


  if( (myrank%2) ) {
      MPI_Isend(sbuf, SIZE, DATATYPE, (newrank+1)%newnprocs, 33, newcomm, &request);
      MPI_Recv(rbuf, SIZE, DATATYPE, (newrank-1+newnprocs)%newnprocs, 33, newcomm, MPI_STATUS_IGNORE);
      MPI_Wait(&request, MPI_STATUS_IGNORE);
  }

 
  for( i=0; i<REPEAT; i++ ) 
    {

    } 

  MPI_Finalize();
  return 0;
}
