
bin_PROGRAMS = simple_mpi simple_mpi.ipm

SIMPLE_MPI_SOURCES = main.c

CC = $(MPICC)

simple_mpi_ipm_SOURCES = $(SIMPLE_MPI_SOURCES)
simple_mpi_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

simple_mpi_SOURCES = $(SIMPLE_MPI_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
