 implicit none

  integer nrow, mcol, irow, jcol, i, j, ndim
  parameter (nrow=3, mcol=2, ndim=2)
  integer p, ierr, row_comm, col_comm, comm2D
  integer Iam, me, row_id, col_id
  integer row_group, row_key, map(0:5)
  data map/2,1,2,1,0,1/
  include "mpif.h"          !! This brings in pre-defined MPI constants, ...
  
  call MPI_Init(ierr)       !! starts MPI
  call MPI_Comm_rank(MPI_COMM_WORLD, Iam, ierr) !! get current process id
  call MPI_Comm_size(MPI_COMM_WORLD, p, ierr) !! get number of processes
  if(Iam .eq. 0) then
     write(*,*)
     write(*,*)'Example of MPI_Comm_split Usage'
     write(*,*)'Split 3x2 grid into 2 different communicators'
     write(*,*)'which correspond to 3 rows and 2 columns.'
     write(*,*)
     write(*,*)'    Iam    irow    jcol  row-id  col-id'
  endif

  irow = Iam/mcol           !! row number
  jcol = mod(Iam, mcol)     !! column number
  comm2D = MPI_COMM_WORLD
  call MPI_Comm_split(comm2D, irow, jcol, row_comm, ierr)
  call MPI_Comm_split(comm2D, jcol, irow, col_comm, ierr)
  
  call MPI_Comm_rank(row_comm, row_id, ierr)
  call MPI_Comm_rank(col_comm, col_id, ierr)
  call MPI_Barrier(MPI_COMM_WORLD, ierr)
  
  write(*,'(9i8)')Iam,irow,jcol,row_id,col_id
  call MPI_Barrier(MPI_COMM_WORLD, ierr)
  
  if(Iam .eq. 0) then
     write(*,*)
     write(*,*)'Next, create more general communicator'
     write(*,*)'which consists of two groups :'
     write(*,*)'Rows 1 and 2 belongs to group 1 and row 3 is group 2'
     write(*,*)
  endif
  
  row_group = Iam/4         ! this expression by no means general
  row_key = Iam - row_group*4 ! group1:0,1,2,3; group2:0,1
  
  call MPI_Comm_split(comm2D, row_group, row_key, &
       row_comm, ierr)
  call MPI_Comm_rank(row_comm, row_id, ierr)
  write(*,'(9i8)')Iam,row_id
  call MPI_Barrier(MPI_COMM_WORLD, ierr)
  
  if(Iam .eq. 0) then
     write(*,*)
     write(*,*)'If two processes have same key, the ranks'
     write(*,*)'of these two processes in the new'
     write(*,*)'communicator will be ordered according'
     write(*,*)'to their order in the old communicator'
     write(*,*)' key = map(Iam); map = (2,1,2,1,0,1)'
     write(*,*)
  endif


  row_group = Iam/4     ! this expression by no means general
  row_key = map(Iam)
  call MPI_Comm_split(comm2D, row_group, row_key, &
       row_comm, ierr)
  call MPI_Comm_rank(row_comm, row_id, ierr)
  call MPI_Barrier(MPI_COMM_WORLD, ierr)
  write(*,'(9i8)')Iam,row_id
  
  call MPI_Finalize(ierr)                        !! let MPI finish up ...
  
end program


