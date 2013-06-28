
IPM_ROOT = ./..

include $(IPM_ROOT)/make_config

LIBIPM   = libipm.a
LIBIPMSO = libipm.so

WRAPMAKER      = $(IPM_ROOT)/bin/make_wrappers
WRAPMAKER_ARGS = $(FUNDERSCORE)

IPM_KEYFILE_MPI     = $(IPM_ROOT)/etc/ipm_key_mpi
IPM_KEYFILE_MPIIO   = $(IPM_ROOT)/etc/ipm_key_mpiio
IPM_KEYFILE_POSIXIO = $(IPM_ROOT)/etc/ipm_key_posixio
IPM_KEYFILE_MEM     = $(IPM_ROOT)/etc/ipm_key_mem
IPM_KEYFILE_CUDA    = $(IPM_ROOT)/etc/ipm_key_cuda
IPM_KEYFILE_CUFFT   = $(IPM_ROOT)/etc/ipm_key_cufft
IPM_KEYFILE_CUBLAS  = $(IPM_ROOT)/etc/ipm_key_cublas

ifeq ($(BUILDOPT_HAVE_MPI), "yes")
IPMFILES_MPI = GEN.wrapper_mpi GEN.calltable_mpi \
	mod_mpi mpi_init mpi_finalize mpi_pcontrol machtopo \
	mpi_fortran 
endif

ifeq ($(BUILDOPT_HAVE_MPIIO), "yes")
IPMFILES_MPIIO = GEN.wrapper_mpiio GEN.calltable_mpiio \
	mod_mpiio
endif

ifeq ($(BUILDOPT_HAVE_POSIXIO), "yes")
IPMFILES_POSIXIO = GEN.wrapper_posixio GEN.fake_posixio \
	GEN.calltable_posixio mod_posixio
endif

ifeq ($(BUILDOPT_HAVE_MEM), "yes")
IPMFILES_MEM = GEN.wrapper_mem GEN.fake_mem GEN.calltable_mem 
endif

ifeq ($(BUILDOPT_HAVE_CALLPATH), "yes")
IPMFILES_CALLPATH = mod_callpath mod_callpath_traverse \
	mod_callpath_cstable
endif

ifeq ($(BUILDOPT_HAVE_KEYHIST), "yes")
IPMFILES_KEYHIST = mod_keyhist #eventgraph
endif

ifeq ($(BUILDOPT_HAVE_PAPI), "yes")
IPMFILES_PAPI = mod_papi
endif



ifeq ($(BUILDOPT_HAVE_SELFMONITOR), "yes")
IPMFILES_SELFMONITOR = mod_selfmonitor
endif

ifeq ($(BUILDOPT_HAVE_PROCCTRL), "yes")
IPMFILES_PROCCTRL = mod_procctrl
endif

ifeq ($(BUILDOPT_HAVE_CLUSTERING), "yes")
IPMFILES_PROCCTRL = mod_clustering
endif

ifeq ($(BUILDOPT_HAVE_OMPTRACEPOINTS), "yes")
IPMFILES_OMPTRACEPOINTS = mod_omptracepoints 
endif

ifeq ($(BUILDOPT_HAVE_CUDA), "yes")
IPMFILES_CUDA = GEN.wrapper_cuda GEN.fake_cuda \
	GEN.calltable_cuda mod_cuda
endif

ifeq ($(BUILDOPT_HAVE_CUFFT), "yes")
IPMFILES_CUFFT = GEN.wrapper_cufft GEN.fake_cufft \
	GEN.calltable_cufft mod_cufft
endif

ifeq ($(BUILDOPT_HAVE_CUBLAS), "yes")
IPMFILES_CUBLAS = GEN.wrapper_cublas GEN.fake_cublas \
	GEN.calltable_cublas mod_cublas
endif


IPMFILES = $(IPMFILES_MPI) $(IPMFILES_MPIIO) $(IPMFILES_POSIXIO) \
	$(IPMFILES_MEM) $(IPMFILES_CALLPATH) $(IPMFILES_PAPI) 	\
	$(IPMFILES_KEYHIST) $(IPMFILES_SELFMONITOR) 		\
	$(IPMFILES_PROCCTRL) $(IPMFILES_CLUSTERING) 		\
	$(IPMFILES_OMPTRACEPOINTS) $(IPMFILES_CUDA) 		\
	$(IPMFILES_CUFFT) $(IPMFILES_CUBLAS)			\
	ipm_core hashtable calltable perfdata jobdata report 	\
	ipm_modules regstack report_xml report_banner ipm_time  \
	ipm_introspect 						\
	memusage ipm_env md5


IPMOBJS = $(addsuffix .o, $(IPMFILES))

IPMOBJS_PIC = $(addsuffix .pic.o, $(IPMFILES))

ifeq ($(BUILDOPT_HAVE_DYNLOAD), "yes")
all : intro $(LIBIPM) $(LIBIPMSO) ipm 
else
all : intro $(LIBIPM) ipm 
endif

intro:
	echo "Building IPM for $(HPCSYS)"

ipm: ipm.c 
	$(CC) -o ipm ipm.c 

$(LIBIPM) : $(IPMOBJS) 
	$(AR) crv libipm.a $(IPMOBJS)
	ranlib libipm.a

$(LIBIPMSO) : $(IPMOBJS_PIC) 
	$(MPICC) -shared -o $(LIBIPMSO) $(IPMOBJS_PIC) -ldl

test : 
	make -C ../test/test.simple_mpi clean
	make -C ../test/test.simple_mpi
	$(MPIRUN) $(MPIRUN_TASKS) ../test/test.simple_mpi/simple_mpi.ipm

utests : utest_callpath #utest_hashtable utest_hashkey utest_hashtable #utest_hashperf utest_regstack 

utest_hashkey : hashkey.c
	gcc -g -DUTEST_HASHKEY -I../include -o $@ $+

utest_hashtable : hashtable.c calltable.c
	gcc -g -DUTEST_HASHTABLE -I../include -o $@ $+

utest_hashperf : hashtable.c calltable.c
	gcc -g -DUTEST_HASHPERF -I../include -o $@ $+

utest_regstack : regstack.c ipm_time.c
	gcc -g -DUTEST_REGSTACK -I../include -o $@ $+

utest_callpath : mod_callpath.c $(LIBIPM)
	$(MPICC) $(CFLAGS) -DUTEST_CALLPATH -I../include -o $@ $+ $(LIBIPM) $(LFLAGS) 

utest_graphs : ipm_graphics.c
	cc -g -DUTEST_PNG $(CFLAGS) -I../include -o $@ $+ $(LFLAGS)

utest_memusage : memusage.c
	gcc -g -DUTEST_MEMUSAGE $(CFLAGS) -I../include -o $@ $+

GEN.wrapper_mpi.c : $(IPM_KEYFILE_MPI) $(WRAPMAKER)
	$(WRAPMAKER) $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MPI)
GEN.calltable_mpi.c : $(IPM_KEYFILE_MPI) $(WRAPMAKER)
	$(WRAPMAKER) $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MPI)

GEN.wrapper_mpiio.c : $(IPM_KEYFILE_MPIIO) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MPIIO)
GEN.calltable_mpiio.c : $(IPM_KEYFILE_MPIIO) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MPIIO)

GEN.wrapper_posixio.c : $(IPM_KEYFILE_POSIXIO) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_POSIXIO)
GEN.calltable_posixio.c : $(IPM_KEYFILE_POSIXIO) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_POSIXIO)
GEN.fake_posixio.c : $(IPM_KEYFILE_POSIXIO) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_POSIXIO)

GEN.wrapper_mem.c : $(IPM_KEYFILE_MEM) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MEM)
GEN.calltable_mem.c : $(IPM_KEYFILE_MEM) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MEM)
GEN.fake_mem.c : $(IPM_KEYFILE_MEM) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MEM)

GEN.wrapper_cuda.c : $(IPM_KEYFILE_CUDA) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUDA)
GEN.calltable_cuda.c : $(IPM_KEYFILE_CUDA) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUDA)
GEN.fake_cuda.c : $(IPM_KEYFILE_CUDA) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUDA)

GEN.wrapper_cufft.c : $(IPM_KEYFILE_CUFFT) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUFFT)
GEN.calltable_cufft.c : $(IPM_KEYFILE_CUFFT) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUFFT)
GEN.fake_cufft.c : $(IPM_KEYFILE_CUFFT) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUFFT)

GEN.wrapper_cublas.c : $(IPM_KEYFILE_CUBLAS) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUBLAS)
GEN.calltable_cublas.c : $(IPM_KEYFILE_CUBLAS) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUBLAS)
GEN.fake_cublas.c : $(IPM_KEYFILE_CUBLAS) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUBLAS)


%.o : %.c 
	$(MPICC) $(CFLAGS) -I../include -c $+ -o $@

%.pic.o : %.c 
	$(MPICC) $(CFLAGS) -fpic -I../include -c $+ -o $@

clean : 
	rm -f *.o *~ *.a *.so ipm 
	rm -f GEN.*.c
	rm -f ../include/GEN.*.h
	rm -f ../lib/$(LIBIPM)
	rm -rf autom4te.cache
	rm -f utest_*
	rm -f ../linkwrap.txt


printenv: 
	@echo "MPICC = $(MPICC)"

