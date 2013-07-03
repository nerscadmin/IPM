      program hello
      implicit none
      integer size, rank, ierr
      double precision val(100)
      double precision test
      integer req
      integer status

      include 'mpif.h'

      call mpi_init(ierr)
      call mpi_comm_size(MPI_COMM_WORLD, size, ierr)
      call mpi_comm_rank(MPI_COMM_WORLD, rank, ierr)

      if (rank .eq. 0) then
         call mpi_gather( MPI_IN_PLACE, 1, MPI_DOUBLE_PRECISION,
     +        val, 1, MPI_DOUBLE_PRECISION, 0, 
     +        MPI_COMM_WORLD, ierr )
      else
         call mpi_gather( test, 1, MPI_DOUBLE_PRECISION,
     +        val, 1, MPI_DOUBLE_PRECISION, 0, 
     +        MPI_COMM_WORLD, ierr )
      end if
      
      call mpi_finalize(ierr)
      end
