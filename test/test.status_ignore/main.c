
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

int main( int argc, char* argv[] )
{
  int myrank, nprocs;
  int val, val2;
  int idx, idx2[2];
  int flag;


  MPI_Request req;
  MPI_Request req2[2];
  MPI_Status stat;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );

  if( nprocs<2 ) {
    fprintf(stderr, "Need at least 2 procs to run this program\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
    return 1;
  }

  /* MPI_STATUS_IGNORE in MPI_Recv */
  switch(myrank) {
  case 0:
    MPI_Send( &val, 1, MPI_INTEGER, 1, 33, MPI_COMM_WORLD);
    break;

  case 1:
    MPI_Recv( &val, 1, MPI_INTEGER, 0, 33, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    break;
  }

  /* MPI_STATUS_IGNORE in MPI_Wait, MPI_Test */
  switch(myrank) {
  case 0:
    MPI_Isend( &val, 1, MPI_INTEGER, 1, 34, MPI_COMM_WORLD, &req);
    MPI_Test( &req, &flag, MPI_STATUS_IGNORE );
    MPI_Wait( &req, MPI_STATUS_IGNORE );

    break;

  case 1:
    MPI_Recv( &val, 1, MPI_INTEGER, 0, 34, MPI_COMM_WORLD, &stat );
    break;
  }

  /* MPI_STATUS_IGNORE in MPI_Waitany, MPI_Testany */
  switch(myrank) {
  case 0:
    MPI_Isend( &val,  1, MPI_INTEGER, 1, 35, MPI_COMM_WORLD, &(req2[0]));
    MPI_Isend( &val2, 1, MPI_INTEGER, 1, 36, MPI_COMM_WORLD, &(req2[1]));
    MPI_Testany( 2, req2, &idx, &flag, MPI_STATUS_IGNORE );
    MPI_Waitany( 2, req2, &idx, MPI_STATUS_IGNORE );
    break;

  case 1:
    MPI_Recv( &val,  1, MPI_INTEGER, 0, 35, MPI_COMM_WORLD, &stat );
    MPI_Recv( &val2, 1, MPI_INTEGER, 0, 36, MPI_COMM_WORLD, &stat );
    break;
  }

  /* MPI_STATUSES_IGNORE in MPI_Waitall, MPI_Testall */
  switch(myrank) {
  case 0:
    MPI_Isend( &val,  1, MPI_INTEGER, 1, 35, MPI_COMM_WORLD, &(req2[0]));
    MPI_Isend( &val2, 1, MPI_INTEGER, 1, 36, MPI_COMM_WORLD, &(req2[1]));
    MPI_Testall( 2, req2, &flag, MPI_STATUSES_IGNORE );
    MPI_Waitall( 2, req2, MPI_STATUSES_IGNORE );
    break;

  case 1:
    MPI_Recv( &val,  1, MPI_INTEGER, 0, 35, MPI_COMM_WORLD, &stat );
    MPI_Recv( &val2, 1, MPI_INTEGER, 0, 36, MPI_COMM_WORLD, &stat );
    break;
  }

  /* MPI_STATUSES_IGNORE in MPI_Waitsome */
  switch(myrank) {
  case 0:
    MPI_Isend( &val,  1, MPI_INTEGER, 1, 35, MPI_COMM_WORLD, &(req2[0]));
    MPI_Isend( &val2, 1, MPI_INTEGER, 1, 36, MPI_COMM_WORLD, &(req2[1]));
    MPI_Testsome( 2, req2, &idx, idx2, MPI_STATUSES_IGNORE );
    MPI_Waitsome( 2, req2, &idx, idx2, MPI_STATUSES_IGNORE );
    break;

  case 1:
    MPI_Recv( &val,  1, MPI_INTEGER, 0, 35, MPI_COMM_WORLD, &stat );
    MPI_Recv( &val2, 1, MPI_INTEGER, 0, 36, MPI_COMM_WORLD, &stat );
    break;
  }




  MPI_Barrier(MPI_COMM_WORLD);
  fprintf(stderr, "%5d: DONE\n", myrank);

  MPI_Finalize();
}
