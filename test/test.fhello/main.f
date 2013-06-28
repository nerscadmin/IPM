      program hello
      implicit none
      integer size, rank, ierr
      integer request, provide
      include 'mpif.h'

      request = MPI_THREAD_SINGLE

      call pmpi_init_thread(request, provide, ierr)
c      call mpi_init(ierr)
      call mpi_comm_size(MPI_COMM_WORLD, size, ierr)
      call mpi_comm_rank(MPI_COMM_WORLD, rank, ierr)
      
      write(6, "(2(a,i3))") " MPI: size = ", size, " rank = ", rank
      call mpi_finalize(ierr)
      end
