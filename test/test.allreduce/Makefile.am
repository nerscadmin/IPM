
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = allreduce allreduce.ipm

ALLREDUCE_SOURCES = main.c

CC = $(MPICC)

allreduce_ipm_SOURCES = $(ALLREDUCE_SOURCES) $(top_srcdir)/include/utest.h
allreduce_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
allreduce_ipm_CFLAGS = -DUTEST

allreduce_SOURCES = $(ALLREDUCE_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
