
bin_PROGRAMS = fork fork.ipm

FORK_SOURCES = main.c

CC = $(MPICC)

fork_ipm_SOURCES = $(FORK_SOURCES)
fork_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

fork_SOURCES = $(FORK_SOURCES)

include  $(top_srcdir)/Makefile.clean-local

