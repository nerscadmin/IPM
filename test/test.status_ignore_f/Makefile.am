
bin_PROGRAMS = fhello fhello.ipm

HELLO_SOURCES = main.f

F77 = $(MPIF77)

fhello_ipm_SOURCES = $(HELLO_SOURCES)
fhello_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

fhello_SOURCES = $(HELLO_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
