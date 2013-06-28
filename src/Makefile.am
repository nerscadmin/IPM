
INCLUDES = -I$(top_srcdir)/include

##############################################################################

IPM_KEYFILE_MPI     = $(top_srcdir)/etc/ipm_key_mpi
IPM_KEYFILE_MPIIO   = $(top_srcdir)/etc/ipm_key_mpiio
IPM_KEYFILE_POSIXIO = $(top_srcdir)/etc/ipm_key_posixio
IPM_KEYFILE_CUDA    = $(top_srcdir)/etc/ipm_key_cuda
IPM_KEYFILE_CUFFT   = $(top_srcdir)/etc/ipm_key_cufft
IPM_KEYFILE_CUBLAS  = $(top_srcdir)/etc/ipm_key_cublas

WRAPMAKER      = $(top_srcdir)/bin/make_wrappers
WRAPMAKER_ARGS = $(IPM_FUNDERSCORE)

##############################################################################

CC = $(MPICC)

# dependencies of report.c
REPORT_DEPS =  $(top_srcdir)/include/ipm.h \
			   $(top_srcdir)/include/ipm_core.h \
			   $(top_srcdir)/include/ipm_types.h \
			   $(top_srcdir)/include/ipm_time.h \
			   $(top_srcdir)/include/hashtable.h \
			   $(top_srcdir)/include/hashkey.h \
			   $(top_srcdir)/include/report.h \
			   $(top_srcdir)/include/regstack.h \
			   $(top_srcdir)/include/calltable.h
			   
# dependencies of ipm_time.c			   
IPM_TIME_DEPS = $(top_srcdir)/include/hashtable.h \
				$(top_srcdir)/include/ipm.h \
				$(top_srcdir)/include/ipm_time.h \
				$(top_srcdir)/include/ipm_debug.h
			   
AM_CFLAGS = -DMPI_STATUS_COUNT=$(IPM_MPISTATUSCOUNT)

IPMFILES_MODULES= 

IPMFILES_MODULES_NODIST= 

IMP_CORE = \
    calltable.c \
    hashkey.c \
    hashtable.c \
    ipm_core.c \
	ipm_env.c \
	ipm_time.c \
	ipm_modules.c \
    jobdata.c \
    md5.c \
	memusage.c \
    perfdata.c \
	regstack.c \
    report.c \
    report_banner.c \
	report_xml.c \
	ipm_introspect.c \
    $(top_srcdir)/include/calltable.h \
    $(top_srcdir)/include/hashtable.h \
    $(top_srcdir)/include/hashkey.h \
    $(top_srcdir)/include/ipm.h \
    $(top_srcdir)/include/ipm_core.h \
    $(top_srcdir)/include/ipm_debug.h \
    $(top_srcdir)/include/ipm_env.h \
	$(top_srcdir)/include/ipm_modules.h \
	$(top_srcdir)/include/ipm_sizes.h \
	$(top_srcdir)/include/ipm_time.h \
	$(top_srcdir)/include/ipm_types.h \
    $(top_srcdir)/include/jobdata.h \
    $(top_srcdir)/include/machtopo.h \
    $(top_srcdir)/include/md5.h \
	$(top_srcdir)/include/memusage.h \
	$(top_srcdir)/include/perfdata.h \
    $(top_srcdir)/include/regstack.h \
    $(top_srcdir)/include/report.h \
    $(top_srcdir)/include/ipm_introspect.h 

###############################################################################

# module section (dependencies)

MPI_MODULE_DIST = mod_mpi.c $(top_srcdir)/include/mod_mpi.h \
	mpi_init.c \
	mpi_finalize.c \
	mpi_pcontrol.c \
	machtopo.c \
	mpi_fortran.c
	
MPI_MODULE_NODIST = GEN.wrapper_mpi.c  \
    GEN.calltable_mpi.c $(top_srcdir)/include/GEN.calltable_mpi.h 

MPIIO_MODULE_DIST = mod_mpiio.c $(top_srcdir)/include/mod_mpiio.h
MPIIO_MODULE_NODIST = GEN.wrapper_mpiio.c \
					  GEN.calltable_mpiio.c \
					  $(top_srcdir)/include/GEN.calltable_mpiio.h \
					  $(top_srcdir)/include/GEN.fproto.mpiio.h
mod_mpiio.c :   $(top_srcdir)/include/mod_mpiio.h \
				$(top_srcdir)/include/GEN.calltable_mpiio.h \
				$(top_srcdir)/include/GEN.fproto.mpiio.h


POSIXIO_MODULE_DIST = mod_posixio.c $(top_srcdir)/include/mod_posixio.h
POSIXIO_MODULE_NODIST = GEN.wrapper_posixio.c $(top_srcdir)/include/GEN.wrapper_posixio.h \
	                    GEN.fake_posixio.c $(top_srcdir)/include/GEN.fake_posixio.h \
						GEN.calltable_posixio.c $(top_srcdir)/include/GEN.calltable_posixio.h

CALLPATH_MODULE_DIST = mod_callpath.c $(top_srcdir)/include/mod_callpath.h \
					   mod_callpath_traverse.c \
					   mod_callpath_evtgraph.c \
					   mod_callpath_cstable.c
CALLPATH_MODULE_NODIST =

SELFMONITOR_MONITOR_DIST= mod_selfmonitor.c \
                          $(top_srcdir)/include/mod_selfmonitor.h
SELFMONITOR_MONITOR_NODIST=

PAPI_MODULE_DIST   = mod_papi.c $(top_srcdir)/include/mod_papi.h
PAPI_MODULE_NODIST =

KEYHIST_MODULE_DIST = mod_keyhist.c $(top_srcdir)/include/mod_keyhist.h 
KEYHIST_MODULE_NODIST = 	  				    

PROCCTRL_DIST  = mod_procctrl.c  $(top_srcdir)/include/mod_procctrl.h
PROCCTRL_NODIST=

CLUSTERING_DIST = mod_clustering.c $(top_srcdir)/include/mod_clustering.h
CLUSTERING_NODIST =

OMPTRACEPOINTS_DIST   = mod_omptracepoints.c mod_omptracepoints_wrap.h $(top_srcdir)/include/mod_omptracepoints.h
OMPTRACEPOINTS_NODIST = 

CUDA_DIST   = mod_cuda.c $(top_srcdir)/include/mod_cuda.h
CUDA_NODIST = GEN.wrapper_cuda.c    $(top_srcdir)/include/GEN.wrapper_cuda.h \
	          GEN.fake_cuda.c       $(top_srcdir)/include/GEN.fake_cuda.h \
	          GEN.calltable_cuda.c  $(top_srcdir)/include/GEN.calltable_cuda.h

CUFFT_DIST   = mod_cufft.c $(top_srcdir)/include/mod_cufft.h
CUFFT_NODIST = GEN.wrapper_cufft.c    $(top_srcdir)/include/GEN.wrapper_cufft.h \
              GEN.fake_cufft.c       $(top_srcdir)/include/GEN.fake_cufft.h \
              GEN.calltable_cufft.c  $(top_srcdir)/include/GEN.calltable_cufft.h

CUBLAS_DIST   = mod_cublas.c $(top_srcdir)/include/mod_cublas.h
CUBLAS_NODIST = GEN.wrapper_cublas.c    $(top_srcdir)/include/GEN.wrapper_cublas.h \
              GEN.fake_cublas.c       $(top_srcdir)/include/GEN.fake_cublas.h \
              GEN.calltable_cublas.c  $(top_srcdir)/include/GEN.calltable_cublas.h



ALL_MODULES_DIST = 	\
					$(CALLPATH_MODULE_DIST) \
					$(KEYHIST_MODULE_DIST) \
                   	$(MEM_MODULE_DIST) \
					$(MPI_MODULE_DIST) \
                   	$(MPIIO_MODULE_DIST) \
                   	$(PAPI_MODULE_DIST) \
					$(POSIXIO_MODULE_DIST) \
					$(PROCCTRL_DIST) \
					$(CLUSTERING_DIST) \
					$(OMPTRACEPOINTS_DIST) \
					$(CUDA_DIST) \
					$(CUFFT_DIST) \
					$(CUBLAS_DIST)
					
					
###############################################################################
# take care here
# tabs before stmts leads to inactive modules

if ENABLE_MPI
  AM_CFLAGS+=-DHAVE_MPI
  IPMFILES_MODULES += $(MPI_MODULE_DIST)
  IPMFILES_MODULES_NODIST += $(MPI_MODULE_NODIST) 
  REPORT_DEPS   += GEN.calltable_mpi.c $(top_srcdir)/include/GEN.calltable_mpi.h
  IPM_TIME_DEPS += GEN.calltable_mpi.c $(top_srcdir)/include/GEN.calltable_mpi.h
endif ENABLE_MPI

if ENABLE_SELF_MONITORING
  AM_CFLAGS+=-DHAVE_SELFMONITOR
  IPMFILES_MODULES += $(SELFMONITOR_MONITOR_DIST)
endif ENABLE_SELF_MONITORING

if ENABLE_MPIIO
  AM_CFLAGS+=-DHAVE_MPIIO
  IPMFILES_MODULES += $(MPIIO_MODULE_DIST)
  IPMFILES_MODULES_NODIST += $(MPIIO_MODULE_NODIST)
  REPORT_DEPS   += GEN.wrapper_mpiio.c GEN.calltable_mpiio.c  $(top_srcdir)/include/GEN.calltable_mpiio.h $(top_srcdir)/include/GEN.fproto.mpiio.h
  IPM_TIME_DEPS += GEN.wrapper_mpiio.c GEN.calltable_mpiio.c  $(top_srcdir)/include/GEN.calltable_mpiio.h $(top_srcdir)/include/GEN.fproto.mpiio.h
endif ENABLE_MPIIO


if ENABLE_POSIXIO
  AM_CFLAGS+=-DHAVE_POSIXIO
  IPMFILES_MODULES += $(POSIXIO_MODULE_DIST)
  IPMFILES_MODULES_NODIST += $(POSIXIO_MODULE_NODIST)		
  REPORT_DEPS += GEN.calltable_posixio.c $(top_srcdir)/include/GEN.calltable_posixio.h 
  IPM_TIME_DEPS += GEN.calltable_posixio.c $(top_srcdir)/include/GEN.calltable_posixio.h
endif

if ENABLE_CALLPATH
  AM_CFLAGS+=-DHAVE_CALLPATH
  IPMFILES_MODULES += $(CALLPATH_MODULE_DIST)
endif

if ENABLE_PAPI
  AM_CFLAGS+=-DHAVE_PAPI
  IPMFILES_MODULES += $(PAPI_MODULE_DIST)
  IPMFILES_MODULES_NODIST += $(PAPI_MODULE_NODIST)
  REPORT_DEPS += $(PAPI_MODULE_DIST)
endif

if ENABLE_KEYHIST
  AM_CFLAGS+=-DHAVE_KEYHIST
  IPMFILES_MODULES += $(KEYHIST_MODULE_DIST)
endif

if ENABLE_PROCCTRL
  AM_CFLAGS+=-DHAVE_PROCCTRL
  IPMFILES_MODULES += $(PROCCTRL_DIST)
  IPMFILES_MODULES_NODIST += $(PROCCTRL_NODIST)
endif

if ENABLE_CLUSTERING
  AM_CFLAGS+=-DHAVE_CLUSTERING
  IPMFILES_MODULES += $(CLUSTERING_DIST)
  IPMFILES_MODULES_NODIST += $(CLUSTERING_NODIST)
endif

if ENABLE_OMPTRACEPOINTS
  AM_CFLAGS+=-DHAVE_OMPTRACEPOINTS
  IPMFILES_MODULES += $(OMPTRACEPOINTS_DIST)
  IPMFILES_MODULES_NODIST += $(OMPTRACEPOINTS_NODIST)
endif

if ENABLE_CUDA
  AM_CFLAGS+=-DHAVE_CUDA
  IPMFILES_MODULES += $(CUDA_DIST)
  IPMFILES_MODULES_NODIST += $(CUDA_NODIST)
  REPORT_DEPS   += GEN.wrapper_cuda.c GEN.calltable_cuda.c GEN.fake_cuda.c
  IPM_TIME_DEPS += GEN.wrapper_cuda.c GEN.calltable_cuda.c GEN.fake_cuda.c
endif

if ENABLE_CUFFT
  AM_CFLAGS+=-DHAVE_CUFFT
  IPMFILES_MODULES += $(CUFFT_DIST)
  IPMFILES_MODULES_NODIST += $(CUFFT_NODIST)
  REPORT_DEPS   += GEN.wrapper_cufft.c GEN.calltable_cufft.c GEN.fake_cufft.c
  IPM_TIME_DEPS += GEN.wrapper_cufft.c GEN.calltable_cufft.c GEN.fake_cufft.c
endif

if ENABLE_CUBLAS
  AM_CFLAGS+=-DHAVE_CUBLAS
  IPMFILES_MODULES += $(CUBLAS_DIST)
  IPMFILES_MODULES_NODIST += $(CUBLAS_NODIST)
  REPORT_DEPS   += GEN.wrapper_cublas.c GEN.calltable_cublas.c GEN.fake_cublas.c
  IPM_TIME_DEPS += GEN.wrapper_cublas.c GEN.calltable_cublas.c GEN.fake_cublas.c
endif

###############################################################################

report.c: $(REPORT_DEPS)

ipm_time.c: $(IPM_TIME_DEPS)

GEN.wrapper_mpi.c : $(IPM_KEYFILE_MPI) $(WRAPMAKER) $(top_srcdir)/include/GEN.calltable_mpi.h
	$(WRAPMAKER) $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MPI)
$(top_srcdir)/include/GEN.calltable_mpi.h: GEN.calltable_mpi.c	
GEN.calltable_mpi.c : $(IPM_KEYFILE_MPI) $(WRAPMAKER)
	$(WRAPMAKER) $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MPI)
	

GEN.wrapper_mpiio.c : $(IPM_KEYFILE_MPIIO) $(WRAPMAKER) \
                      $(top_srcdir)/include/GEN.calltable_mpi.h \
                      $(top_srcdir)/include/GEN.calltable_mpiio.h
	$(WRAPMAKER) -nofort  $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MPIIO)
	
$(top_srcdir)/include/GEN.calltable_mpiio.h : GEN.calltable_mpiio.c	
GEN.calltable_mpiio.c : $(IPM_KEYFILE_MPIIO) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_MPIIO)

$(top_srcdir)/include/GEN.wrapper_posixio.h : GEN.wrapper_posixio.c	
GEN.wrapper_posixio.c : $(IPM_KEYFILE_POSIXIO) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_POSIXIO)
$(top_srcdir)/include/GEN.calltable_posixio.h : GEN.calltable_posixio.c	
GEN.calltable_posixio.c : $(IPM_KEYFILE_POSIXIO) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_POSIXIO)
$(top_srcdir)/include/GEN.fake_posixio.h : GEN.fake_posixio.c	
GEN.fake_posixio.c : $(IPM_KEYFILE_POSIXIO) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_POSIXIO)


### cuda dependencies
mod_cuda.c: $(top_srcdir)/include/GEN.calltable_cuda.h
	
$(top_srcdir)/include/GEN.wrapper_cuda.h : GEN.wrapper_cuda.c	
GEN.wrapper_cuda.c : $(IPM_KEYFILE_CUDA) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUDA)

$(top_srcdir)/include/GEN.calltable_cuda.h : GEN.calltable_cuda.c	
GEN.calltable_cuda.c : $(IPM_KEYFILE_CUDA) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUDA)

$(top_srcdir)/include/GEN.fake_cuda.h : GEN.fake_cuda.c	
GEN.fake_cuda.c : $(IPM_KEYFILE_CUDA) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUDA)	
	
### cufft dependencies
mod_cufft.c: $(top_srcdir)/include/GEN.calltable_cufft.h
    
$(top_srcdir)/include/GEN.wrapper_cufft.h : GEN.wrapper_cufft.c   
GEN.wrapper_cufft.c : $(IPM_KEYFILE_CUFFT) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUFFT)

$(top_srcdir)/include/GEN.calltable_cufft.h : GEN.calltable_cufft.c   
GEN.calltable_cufft.c : $(IPM_KEYFILE_CUFFT) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUFFT)

$(top_srcdir)/include/GEN.fake_cufft.h : GEN.fake_cufft.c 
GEN.fake_cufft.c : $(IPM_KEYFILE_CUFFT) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUFFT)   

### cublas dependencies
mod_cublas.c: $(top_srcdir)/include/GEN.calltable_cublas.h
    
$(top_srcdir)/include/GEN.wrapper_cublas.h : GEN.wrapper_cublas.c   
GEN.wrapper_cublas.c : $(IPM_KEYFILE_CUBLAS) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUBLAS)

$(top_srcdir)/include/GEN.calltable_cublas.h : GEN.calltable_cublas.c   
GEN.calltable_cublas.c : $(IPM_KEYFILE_CUBLAS) $(WRAPMAKER)
	$(WRAPMAKER) -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUBLAS)

$(top_srcdir)/include/GEN.fake_cublas.h : GEN.fake_cublas.c 
GEN.fake_cublas.c : $(IPM_KEYFILE_CUBLAS) $(WRAPMAKER)
	$(WRAPMAKER) -dummy -nofort $(WRAPMAKER_ARGS) $(IPM_KEYFILE_CUBLAS)   

###############################################################################

lib_LTLIBRARIES = libipm.la

libipm_la_CFLAGS = $(AM_CFLAGS)

libipm_la_SOURCES = $(IMP_CORE) $(IPMFILES_MODULES)
nodist_libipm_la_SOURCES = $(IPMFILES_MODULES_NODIST)
EXTRA_DIST = $(ALL_MODULES_DIST)
	
###############################################################################
	
clean-local:
	-rm -f GEN.*
	-rm -f  $(top_srcdir)/include/GEN.*
	
