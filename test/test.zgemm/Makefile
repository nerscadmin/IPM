
EXTRADEP=fortran_thunking.o



#LFLAGS = -L$(MKL_LIBDIR) -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5
#LFLAGS = $(CUDA_LIB64) -lcublas 
LFLAGS = $(CUDA_LIB64) -lcublas -L$(MKL_LIBDIR) -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5

MPIF90OPTS=-mp

include ../Makefile_f90

fortran_thunking.o : fortran_thunking.c fortran.h
	nvcc -c $(CUDA_INCLUDE) fortran_thunking.c

