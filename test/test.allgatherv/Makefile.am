
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = allgatherv allgatherv.ipm

ALLGATHERV_SOURCES = main.c

CC = $(MPICC)

allgatherv_ipm_SOURCES = $(ALLGATHERV_SOURCES) $(top_srcdir)/include/utest.h
allgatherv_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
allgatherv_ipm_CFLAGS = -DUTEST

allgatherv_SOURCES = $(ALLGATHERV_SOURCES)

include  $(top_srcdir)/Makefile.clean-local

