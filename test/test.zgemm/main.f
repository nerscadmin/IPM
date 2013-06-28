      program hello
      include 'mpif.h'

      integer ierr, size, rank
      integer i, j
      
      call mpi_init(ierr)
      call mpi_comm_size(MPI_COMM_WORLD, size, ierr)
      call mpi_comm_rank(MPI_COMM_WORLD, rank, ierr)

      call cublas_init(0)

      do size = 200, 5000, 120

         call testzgemm(size)

      end do

      call mpi_finalize(ierr)

      end




      subroutine testzgemm(size)
      include 'mpif.h'

      integer dim1, dim2, dim3
      
      double complex, dimension(:, :), allocatable :: A
      double complex, dimension(:, :), allocatable :: B
      double complex, dimension(:, :), allocatable :: C

      integer size, i, j

      double complex alpha, beta
      double complex sum
      double precision start, stop
      alpha = 1.0d0
      beta = 0.0d0
      

      dim1 = size
      dim2 = size
      dim3 = size

      allocate ( A(dim1,dim2) )
      allocate ( B(dim2,dim3) )
      allocate ( C(dim1,dim3) )

      call srand(86456)
      do i = 1, dim1
         do j = 1, dim2
            A(i, j) = 0.1d-3 * DBLE(i*j+j)
         enddo
      enddo
      do i = 1, dim2
         do j = 1, dim3
            B(i, j) = 0.1d-3 * DBLE(i+j*j)
         enddo
      enddo


      call mpi_pcontrol(1, "cublas_zgemm"//char(0))
      start = mpi_wtime()
      call cublas_zgemm('N','N',dim1,
     +     dim3,dim2,alpha,A,dim1,B,dim2,beta,C,dim1)
      stop = mpi_wtime()
      call mpi_pcontrol(-1, "cublas_zgemm"//char(0))

      sum = 0.0d1
      do i = 1, dim1
         sum = sum + C(i,i)
      enddo      

      write(*,*) 'CUBLAS: ', dim1, 8.0d0*DBLE(dim1)*
     c     DBLE(dim2)*DBLE(dim3)/(1.0e9*(stop-start)),
     c     sum
      
      call mpi_pcontrol(1, "zgemm"//char(0))
      start = mpi_wtime()
      call zgemm('N','N',dim1,
     +     dim3,dim2,alpha,A,dim1,B,dim2,beta,C,dim1)
      stop = mpi_wtime()
      call mpi_pcontrol(-1, "zgemm"//char(0))

      sum = 0.0d1
      do i = 1, dim1
         sum = sum + C(i,i)
      enddo            

      write(*,*) 'MKL   : ', dim1, 8.0d0*DBLE(dim1)*
     c     DBLE(dim2)*DBLE(dim3)/(1.0e9*(stop-start)),
     c     sum

      do i = 1, dim1
         do j = 1, dim3
            C(i, j) = 0.0d1
         enddo
      enddo

      call mpi_pcontrol(1, "myzgemm"//char(0))
      start = mpi_wtime()
      call myzgemm('N','N',dim1,
     +     dim3,dim2,alpha,A,dim1,B,dim2,beta,C,dim1)
      stop = mpi_wtime()
      call mpi_pcontrol(-1, "myzgemm"//char(0))

      sum = 0.0d1
      do i = 1, dim1
         sum = sum + C(i,i)
      enddo            

      write(*,*) 'BOTH  : ', dim1, 8.0d0*DBLE(dim1)*
     c     DBLE(dim2)*DBLE(dim3)/(1.0e9*(stop-start)),
     c     sum


      deallocate ( A )
      deallocate ( B ) 
      deallocate ( C ) 
      
      end subroutine testzgemm


      subroutine myzgemm(TRANSA,TRANSB,M,N,K,ALPHA,A,LDA,B,
     +     LDB,BETA,C,LDC)
      
      DOUBLE COMPLEX ALPHA,BETA
      INTEGER K,LDA,LDB,LDC,M,N
      CHARACTER TRANSA,TRANSB
      
      DOUBLE COMPLEX A(LDA,*),B(LDB,*),C(LDC,*)

      call my_zgemm1(TRANSA,TRANSB,6*M/8,N,K,ALPHA,A,LDA,B,
     +     LDB,BETA,C,LDC)

      call zgemm(TRANSA,TRANSB,2*M/8,N,K,ALPHA,A(6*M/8+1:),LDA,B,
     +     LDB,BETA,C(6*M/8+1:),LDC)
      
      call my_zgemm2(TRANSA,TRANSB,6*M/8,N,K,ALPHA,A,LDA,B,
     +     LDB,BETA,C,LDC)



      end subroutine myzgemm
