
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#define SIZE      10
#define DATATYPE  MPI_DOUBLE
#define ROOT      0
#define REPEAT    5
#define FILENAME  "test.out"

int main( int argc, char* argv[] )
{
  int i, myrank, nprocs;
  char *buf;
  int dsize;

  MPI_File fh;
  MPI_Status status;
  MPI_Info info;

  MPI_Init( &argc, &argv );

  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
  MPI_Type_size(DATATYPE, &dsize);

  buf=(char*)malloc(SIZE*dsize);
  for( i=0; i<REPEAT; i++ )
    {
      MPI_File_open( MPI_COMM_SELF, FILENAME, MPI_MODE_CREATE | MPI_MODE_RDWR,
		     MPI_INFO_NULL, &fh );

      MPI_File_get_info(fh, &info);

      MPI_File_set_view(fh, 0, DATATYPE, DATATYPE, "native", MPI_INFO_NULL);
      MPI_File_write(fh, buf, SIZE, DATATYPE, &status);
      
      MPI_File_close(&fh);
    }

  free(buf);
  MPI_Finalize();
}
