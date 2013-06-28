
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = bcast bcast.ipm

BCAST_SOURCES = main.c

CC = $(MPICC)

bcast_ipm_SOURCES = $(BCAST_SOURCES) $(top_srcdir)/include/utest.h
bcast_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
bcast_ipm_CFLAGS = -DUTEST

bcast_SOURCES = $(BCAST_SOURCES)


include  $(top_srcdir)/Makefile.clean-local
