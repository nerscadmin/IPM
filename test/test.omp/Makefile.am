
bin_PROGRAMS = omp omp.ipm

OMP_SOURCES = main.c

CC = $(MPICC)

omp_ipm_SOURCES = $(OMP_SOURCES)
omp_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

omp_SOURCES = $(OMP_SOURCES)

include  $(top_srcdir)/Makefile.clean-local

