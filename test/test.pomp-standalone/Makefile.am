
bin_PROGRAMS = pomp pomp.ipm

POMP_SOURCES = main.c opari_omp.h pomp_lib.h

CC = $(MPICC)

pomp_ipm_SOURCES = $(POMP_SOURCES)
pomp_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
pomp_ipm_CFLAGS  =-fopenmp
pomp_ipm_LFLAGS  =-fopenmp

pomp_SOURCES = $(POMP_SOURCES)
pomp_CFLAGS  =-fopenmp
pomp_LFLAGS  =-fopenmp


include  $(top_srcdir)/Makefile.clean-local
