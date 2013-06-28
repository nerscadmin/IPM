
bin_PROGRAMS = status_ignore status_ignore.ipm

STATUS_IGNORE_SOURCES = main.c

CC = $(MPICC)

status_ignore_ipm_SOURCES = $(STATUS_IGNORE_SOURCES)
status_ignore_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

status_ignore_SOURCES = $(STATUS_IGNORE_SOURCES)

include  $(top_srcdir)/Makefile.clean-local

