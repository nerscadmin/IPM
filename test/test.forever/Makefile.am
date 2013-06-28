
bin_PROGRAMS = forever forever.ipm

FOREVER_SOURCES = main.c

CC = $(MPICC)

forever_ipm_SOURCES = $(FOREVER_SOURCES)
forever_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

forever_SOURCES = $(FOREVER_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
