
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define SIZE      10
#define DATATYPE  MPI_BYTE
#define REPEAT    5

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int *rcnt, *rdpl, rsize;
  int dsize;

  MPI_Init( &argc, &argv );
  
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Type_size(DATATYPE, &dsize);

  rcnt = (int*) malloc(nprocs*(sizeof(int)));
  rdpl = (int*) malloc(nprocs*(sizeof(int)));

  for( i=0; i<nprocs; i++ )
    {
      rcnt[i]=SIZE*(i+1);
      rdpl[i]=SIZE*i*(i+1)/2;
    }

  rsize=0; for( i=0; i<nprocs; i++ ) rsize+=rcnt[i];

  sbuf=(char*)malloc(SIZE*dsize*(myrank+1));
  rbuf=(char*)malloc(SIZE*dsize*rsize);

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Allgatherv( sbuf, SIZE*(myrank+1), DATATYPE,
		      rbuf, rcnt, rdpl, DATATYPE,
		      MPI_COMM_WORLD );
    }

  MPI_Finalize();
}
