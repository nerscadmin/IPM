
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = gatherv gatherv.ipm

GATHERV_SOURCES = main.c

CC = $(MPICC)

gatherv_ipm_SOURCES = $(GATHERV_SOURCES) $(top_srcdir)/include/utest.h
gatherv_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a
gatherv_ipm_CFLAGS = -DUTEST

gatherv_SOURCES = $(GATHERV_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
