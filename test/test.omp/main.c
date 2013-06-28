
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

#define REPEAT 1
//#define IPM 1

void do_some_flops()
{
  int i, j;
  double a;
  
  a=1.01;
  for(i=1; i<1000; i++) {
    for(j=1; j<1000; j++ ) {
      a+=(double)(i)+(double)(i)/(double)(j);
    }
  }
  
  fprintf(stdout, "%5.3f\n", a);
}


void func_with_omp()
{
  MPI_Pcontrol(1, "func_with_omp");

  do_some_flops();
  
  /* manually call the trace functions if not
     using the PGI compiler */
#if defined(IPM) && !defined(__PGI)
  _mp_trace_parallel_enter();
#endif
#pragma omp parallel
  {
#if defined(IPM) && !defined(__PGI)
    _mp_trace_parallel_begin();
#endif

    if(omp_get_thread_num()==0) {
      sleep(1.0);
    }
#if defined(IPM) && !defined(__PGI)
    _mp_trace_parallel_end();
#endif
  }
#if defined(IPM) && !defined(__PGI)
  _mp_trace_parallel_exit();
#endif
  
  MPI_Pcontrol(-1, "func_with_omp");
}


int main( int argc, char* argv[] )
{
  int i;
  int myrank, nprocs;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );

  for( i=0; i<REPEAT; i++ )
    {
      func_with_omp();
    }

  MPI_Finalize();
}
