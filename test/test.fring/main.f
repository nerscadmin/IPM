C************************************************************
C     FILE: ring.f
C     
C     DESCRIPTION:
C     This example program uses MPI blocking and nonblocking
C     point-to-point communication calls to send and receive
C     message in the ring where node wirh rank i sends a message
C     to the node with rank i+1 and receives a message from the node 
C     with rank i-1.
C     
C     AUTHOR: Roslyn Leibensperger (MPL version)             1/15/93
C     Xianneng Shen converted from MPL to MPI       11/20/94
C**************************************************************
      program ring
c     
      implicit none
      include 'mpif.h'
c     
      integer ntasks, taskid, right, left, inmsg, outmsg, mtype,
     .     msgid, rbytes, sbytes, i
      integer ierr,status(MPI_STATUS_SIZE),tag,requests(2)
      
      data outmsg /0/
      data i /1/

      
      tag = 33

c     
c     learn number of tasks in partition and task ID 
      call mpi_init(ierr)
      call mpi_comm_size(MPI_COMM_WORLD,ntasks,ierr)
      call mpi_comm_rank(MPI_COMM_WORLD,taskid,ierr)
      
c     compute source and destination for messages
      if (taskid.eq.0) then 
         left = ntasks - 1
      else 
         left = taskid - 1 
      endif
      if (taskid.eq.ntasks-1) then 
         right = 0
      else 
         right = taskid + 1
      endif
c     
      outmsg = 2
      do while (outmsg .ne. -1)
         mtype = i
c     node 0 queries user for message, sends it to the right,
c     then waits for its return
         if (taskid .eq. 0) then
            write (*,*) 'Enter integer value to be passed along ring'
            write (*,*) 'A value of -1 ends the program'
c     read (*,*) outmsg
            call mpi_send(outmsg,1,MPI_INTEGER,right,tag,
     &           MPI_COMM_WORLD,ierr)
            call mpi_irecv(inmsg,1,MPI_INTEGER,left,tag,
     &           MPI_COMM_WORLD,requests(1),ierr)
            call mpi_wait(requests(1),status,ierr)
            write (*,*) 'node 0 received message ',
     .           'content is ', inmsg
c     the rest of the nodes in the group read the message and pass it on
         else
            call mpi_recv (inmsg,1,MPI_INTEGER,left,tag,
     &           MPI_COMM_WORLD, status,ierr)
            outmsg = inmsg
            call mpi_isend (outmsg,1,MPI_INTEGER,right,tag,
     &           MPI_COMM_WORLD, requests(2),ierr)
            call mpi_wait (requests(2),status,ierr)
            write (*,*) taskid, 'processed message ', mtype,
     .           'content is ', outmsg
         end if       
         i = i + 1
         outmsg = outmsg -1
      end do

      call mpi_finalize(ierr)
      end
      
