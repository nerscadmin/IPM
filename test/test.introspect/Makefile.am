
bin_PROGRAMS = introspect.ipm

INTROSPECT_SOURCES = main.c

CC = $(MPICC)

introspect_ipm_SOURCES = $(INTROSPECT_SOURCES)
introspect_ipm_LDADD   = $(top_builddir)/src/.libs/libipm.a

introspect_SOURCES = $(INTROSPECT_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
