
bin_PROGRAMS = pmon-dgemm pmon-dgemm.ipm

PMON_SOURCES = pmon-dgemm.c

CC = $(MPICC)
CFLAGS+=-O3 -fopenmp -Wall -Wextra -g
pmon_ipm_SOURCES = $(PMON_SOURCES)

pmon_dgemm_ipm_LDADD   = -L$(top_builddir)/src/.libs/ -lipm

pmon_SOURCES = $(PMON_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
