
bin_PROGRAMS = pmon pmon.ipm

PMON_SOURCES = pmon.c

CC = $(MPICC)
AM_CFLAGS=  -Wall -Wextra
pmon_ipm_SOURCES = $(PMON_SOURCES)

pmon_ipm_LDADD   = -L$(top_builddir)/src/.libs/ -lipm

pmon_SOURCES = $(PMON_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
