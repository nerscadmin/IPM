
bin_PROGRAMS = fring fring.ipm

FRING_SOURCES = main.f

F77 = $(MPIFC)

fring_ipm_SOURCES = $(FRING_SOURCES)
fring_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

fring_SOURCES = $(FRING_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
