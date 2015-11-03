
WRAPPER_TEMPLATES = $(wildcard $(srcdir)/fake_*.c $(srcdir)/wrap_*.c)
WRAPPER_HELPER = $(wildcard $(srcdir)/ipm_key_*)

#WRAPPER_TEMPLATES = \
	fake_cublas_c.c  fake_cuda_c.c  fake_cufft_c.c  fake_posixio_c.c \
	wrap_cublas_c.c  wrap_cufft_c.c  wrap_mpi_f.c    wrap_mpiio_f.c \
	wrap_cuda_c.c    wrap_mpi_c.c    wrap_mpiio_c.c  wrap_posixio_c.c
#WRAPPER_HELPER = \
	ipm_key_cublas ipm_key_cufft ipm_key_mpi ipm_key_posixio \
	ipm_key_cuda   ipm_key_mem   ipm_key_mpiio

EXTRA_DIST = $(WRAPPER_HELPER) $(WRAPPER_TEMPLATES)
sysconf_DATA = $(WRAPPER_HELPER)


