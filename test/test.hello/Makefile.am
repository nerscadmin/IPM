
bin_PROGRAMS = hello hello.ipm

HELLO_SOURCES = main.c

CC = $(MPICC)

hello_ipm_SOURCES = $(HELLO_SOURCES)
hello_ipm_LDADD   = $(top_srcdir)/src/.libs/libipm.a

hello_SOURCES = $(HELLO_SOURCES)

include  $(top_srcdir)/Makefile.clean-local
