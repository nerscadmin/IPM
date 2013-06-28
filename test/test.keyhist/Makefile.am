
bin_PROGRAMS = keyhist keyhist.ipm

KEYHIST_SOURCES = main.c

CC = $(MPICC)

keyhist_ipm_SOURCES = $(KEYHIST_SOURCES)
keyhist_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

keyhist_SOURCES = $(KEYHIST_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
