
bin_PROGRAMS = posix-read posix-read.ipm

POSIX_READ_SOURCES = main.c

CC = $(MPICC)

posix_read_ipm_SOURCES = $(POSIX_READ_SOURCES)
posix_read_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

posix_read_SOURCES = $(POSIX_READ_SOURCES)


include  $(top_srcdir)/Makefile.clean-local
