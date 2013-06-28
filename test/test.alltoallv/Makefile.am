
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = alltoallv alltoallv.ipm

ALLTOALLV_SOURCES = main.c

CC = $(MPICC)

alltoallv_ipm_SOURCES = $(ALLTOALLV_SOURCES) $(top_srcdir)/include/utest.h
alltoallv_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
alltoallv_ipm_CFLAGS = -DUTEST

alltoallv_SOURCES = $(ALLTOALLV_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
