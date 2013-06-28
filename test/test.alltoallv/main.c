
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define SIZE      1
#define DATATYPE  MPI_BYTE
#define REPEAT    1

int main( int argc, char* argv[] )
{
  int i, j;
  int myrank, nprocs;
  char *sbuf,  *rbuf;
  int *scnt, *rcnt;
  int *sdpl, *rdpl;
  int dsize;
  int ssize, rsize;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Type_size(DATATYPE, &dsize);

  scnt = malloc( sizeof(int)*nprocs );
  sdpl = malloc( sizeof(int)*nprocs );
  rcnt = malloc( sizeof(int)*nprocs );
  rdpl = malloc( sizeof(int)*nprocs );
  
  for( i=0; i<nprocs; i++ )
    {
      scnt[i]=SIZE*(i+1)*(myrank+1);
      rcnt[i]=SIZE*(i+1)*(myrank+1);
      sdpl[i]=SIZE*((i*(i+1))/2)*(myrank+1);
      rdpl[i]=SIZE*((i*(i+1))/2)*(myrank+1);
    }
  
  ssize=0; for(i=0; i<nprocs; i++) ssize+=scnt[i];
  rsize=0; for(i=0; i<nprocs; i++) rsize+=rcnt[i];
  
  sbuf = (char*) malloc( SIZE*dsize*ssize );
  rbuf = (char*) malloc( SIZE*dsize*rsize );

  for( i=0; i<REPEAT; i++ )
    {
      MPI_Alltoallv( sbuf, scnt, sdpl, DATATYPE,
		     rbuf, rcnt, rdpl, DATATYPE,
		     MPI_COMM_WORLD );
    }

  fprintf(stdout, "DONE (rank %d)!\n", myrank);
  
  MPI_Finalize();
}
