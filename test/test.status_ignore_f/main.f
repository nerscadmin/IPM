      program hello
      implicit none
      integer size, rank, ierr
      double precision val(100)
      integer req
      integer status

      include 'mpif.h'



      call mpi_init(ierr)
      call mpi_comm_size(MPI_COMM_WORLD, size, ierr)
      call mpi_comm_rank(MPI_COMM_WORLD, rank, ierr)

! MPI_STATUS_IGNORE in mpi_recv

      if (size < 2 ) then
         write(*,*) "Need at least 2 procs for this program"
         call mpi_abort(1)
      end  if
      
      write(6, "(2(a,i3))") " MPI: size = ", size, " rank = ", rank


      if (rank .eq. 0) then
         call mpi_send( val, 100, MPI_DOUBLE_PRECISION, 1, 33,
     +        MPI_COMM_WORLD, ierr )
      end if
      
      
      if (rank .eq. 1) then
         call mpi_recv( val, 100, MPI_DOUBLE_PRECISION, 0, 33,
     +        MPI_COMM_WORLD, MPI_STATUS_IGNORE, ierr )         
      end if

! MPI_STATUS_IGNORE in mpi_wait

      if (rank .eq. 0) then
         call mpi_isend( val, 100, MPI_DOUBLE_PRECISION, 1, 33,
     +        MPI_COMM_WORLD, req, ierr )
         call mpi_wait( req, MPI_STATUS_IGNORE, ierr )
      end if
      
      
      if (rank .eq. 1) then
         call mpi_recv( val, 100, MPI_DOUBLE_PRECISION, 0, 33,
     +        MPI_COMM_WORLD, MPI_STATUS_IGNORE, ierr )         
      end if
      
   
         

      call mpi_finalize(ierr)
      end
