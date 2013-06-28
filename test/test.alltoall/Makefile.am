
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = alltoall alltoall.ipm

ALLTOALL_SOURCES = main.c

CC = $(MPICC)

alltoall_ipm_SOURCES = $(ALLTOALL_SOURCES) $(top_srcdir)/include/utest.h
alltoall_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
alltoall_ipm_CFLAGS = -DUTEST

alltoall_SOURCES = $(ALLTOALL_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
