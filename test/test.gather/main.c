
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>


#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define ROOT      1
#define REPEAT    1

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int dsize;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  rbuf=0;
  if( myrank==ROOT ) {
    rbuf=(char*)malloc(SIZE*nprocs*dsize);
  }

  sbuf=(char*)malloc(SIZE*dsize);

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Gather( sbuf, SIZE, DATATYPE,
		  rbuf, (myrank==ROOT?SIZE:0), DATATYPE,
		  ROOT, MPI_COMM_WORLD );
    } 
   
  MPI_Finalize();
}
