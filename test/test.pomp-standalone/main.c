#ifdef _POMP
#  undef _POMP
#endif
#define _POMP 200110

#include "main.c.opari.inc"
#line 1 "main.c"

#include <omp.h>
#include <stdio.h>


int main( int argc, char* argv[])
{

POMP_Parallel_fork(&omp_rd_1);
#line 7 "main.c"
#pragma omp parallel POMP_DLIST_00001
{ POMP_Parallel_begin(&omp_rd_1);
#line 8 "main.c"
  {
    fprintf(stderr, "Thread %d of %d\n", 
	    omp_get_thread_num(), omp_get_num_threads());
    if( omp_get_thread_num()==0 )
      sleep(1);
  }
POMP_Barrier_enter(&omp_rd_1);
#pragma omp barrier
POMP_Barrier_exit(&omp_rd_1);
POMP_Parallel_end(&omp_rd_1); }
POMP_Parallel_join(&omp_rd_1);
#line 11 "main.c"

  return 0;
}
