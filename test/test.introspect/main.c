
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#include "../../include/ipm_introspect.h"

void foo()
{
  int i;
  MPI_Pcontrol(1, "foo");

  pia_regid_t rid, rid2;
  rid = pia_current_region();
  fprintf(stderr, "this is region:%d\n", rid);

  for(i=0; i<100; i++ ) {
    MPI_Barrier(MPI_COMM_WORLD);
  }

  sleep(1.0);

  MPI_Pcontrol(-1, "foo");
}

void bar()
{
  MPI_Pcontrol(1, "bar");
  foo();

  pia_regid_t rid, rid2;
  rid = pia_current_region();
  fprintf(stderr, "this is region:%d\n", rid);

  rid2 = pia_child_region(rid);
  fprintf(stderr, "It's child is region:%d\n", rid2);

  MPI_Pcontrol(-1, "bar");
}

int main( int argc, char* argv[] )
{
  int myrank, nprocs;
  pia_regid_t rid;
  pia_regdata_t rdata;


  MPI_Init( &argc, &argv);

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );

  foo();
  foo();

  rid = pia_find_region_by_name("foo");
  fprintf(stderr, "region foo is %d\n", rid);

  pia_get_region_data(&rdata, rid); 

  fprintf(stderr, "%d execc\n", rdata.count);
  fprintf(stderr, "%f wallt\n", rdata.wtime);
  fprintf(stderr, "%f mpit\n", rdata.mtime);
  fprintf(stderr, "%s\n", rdata.name);

  char *act = "MPI_Barrier";

  pia_act_t aid  = pia_find_activity_by_name(act);
  pia_actdata_t adata;

  pia_init_activity_data(&adata);
  pia_get_activity_data(&adata, aid);

  fprintf(stderr, "%s happened %d times\n", act, adata.ncalls);
  
  MPI_Finalize();
  return 0;
}
