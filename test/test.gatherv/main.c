
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define ROOT      1
#define REPEAT    5

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int *rcnt, *rdpl;
  int dsize;

  MPI_Init( &argc, &argv );
  
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  rbuf=0; 
  rcnt=0; rdpl=0;
  if( myrank==ROOT )
    {
      rbuf=(char*)malloc(SIZE*dsize * ((nprocs*(nprocs+1))/2+nprocs) );
      rcnt=(int*) malloc(sizeof(int)*nprocs);
      rdpl=(int*) malloc(sizeof(int)*nprocs);

      for( i=0; i<nprocs; i++ )
	{
	  rcnt[i] = SIZE*(i+1);
	  rdpl[i] = SIZE*(i*((i+1)/2)+i+1);
	}
    }
  sbuf=(char*)malloc(SIZE*dsize * (myrank+1) );


  for( i=0; i<REPEAT; i++ )
    {
      MPI_Gatherv( sbuf, SIZE*(myrank+1), DATATYPE, 
		   rbuf, rcnt, rdpl, DATATYPE, ROOT, MPI_COMM_WORLD );
    }

  MPI_Finalize();
}
