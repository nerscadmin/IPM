
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = gather gather.ipm

GATHER_SOURCES = main.c

CC = $(MPICC)

gather_ipm_SOURCES = $(GATHER_SOURCES) $(top_srcdir)/include/utest.h
gather_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
gather_ipm_CFLAGS = -DUTEST

gather_SOURCES = $(GATHER_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
