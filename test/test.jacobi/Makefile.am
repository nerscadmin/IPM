
bin_PROGRAMS = jacobi jacobi.ipm

JACOBI_SOURCES = jacobi.c

CC = $(MPICC)

jacobi_ipm_SOURCES = $(JACOBI_SOURCES)
jacobi_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

jacobi_SOURCES = $(JACOBI_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
