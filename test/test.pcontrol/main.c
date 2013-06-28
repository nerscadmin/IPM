
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define ROOT      0
#define REPEAT    10

void xxx()
{
  MPI_Pcontrol(1,  "xxx");
  MPI_Pcontrol(-1, "xxx");
}

void foo()
{
  int nprocs, i;
  MPI_Pcontrol(1,  "foo");
  
  for( i=0; i<1000; i++ ) {
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  }

  MPI_Pcontrol(-1, "foo");
}

void bar1()
{
  int myrank, i;
  MPI_Pcontrol(1,  "bar1");
  foo();

  for( i=0; i<10; i++ ) {
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  }

  MPI_Pcontrol(-1, "bar1");
}

void bar2()
{
  int myrank, i;
  MPI_Pcontrol(1,  "bar2");
  foo();

  for( i=0; i<20; i++ ) {
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  }

  MPI_Pcontrol(-1, "bar2");
}



int main( int argc, char* argv[] )
{
  int i;
  int myrank, nprocs;
  char *buf;
  int dsize;
  double a=1.01;

  MPI_Init( &argc, &argv );

  MPI_Pcontrol(1, "main");

  sleep(1.0);



  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  PMPI_Type_size(DATATYPE, &dsize);

  bar1();
  bar2();

  MPI_Pcontrol(-1, "main");

  MPI_Finalize();
}
