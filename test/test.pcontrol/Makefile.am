
bin_PROGRAMS = pcontrol pcontrol.ipm

PCONTROL_SOURCES = main.c

CC = $(MPICC)

pcontrol_ipm_SOURCES = $(PCONTROL_SOURCES)
pcontrol_ipm_LDADD   = $(top_builddir)/src/.libs/libipm.a

pcontrol_SOURCES = $(PCONTROL_SOURCES)

include  $(top_srcdir)/Makefile.clean-local

