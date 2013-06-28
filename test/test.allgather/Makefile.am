
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = allgather allgather.ipm

ALLGATHER_SOURCES = main.c

CC = $(MPICC)

allgather_ipm_SOURCES = $(ALLGATHER_SOURCES) $(top_srcdir)/include/utest.h
allgather_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
allgather_ipm_CFLAGS = -DUTEST

allgather_SOURCES = $(ALLGATHER_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
