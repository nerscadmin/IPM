
#ifndef IPM_MODULES_H_INCLUDED
#define IPM_MODULES_H_INCLUDED

#include "ipm_sizes.h"
#include "regstack.h"


#define IPM_MODULE_MPI               0
#define IPM_MODULE_MPIIO             1
#define IPM_MODULE_POSIXIO           2
#define IPM_MODULE_OMPTRACEPOINTS    3
#define IPM_MODULE_CUDA              4
#define IPM_MODULE_CUFFT             5
#define IPM_MODULE_CUBLAS            6
#define IPM_MODULE_PAPI              7
#define IPM_MODULE_SELFMONITOR       8
#define IPM_MODULE_CALLPATH          9
#define IPM_MODULE_KEYHIST          10
#define IPM_MODULE_PROCCTRL         11
#define IPM_MODULE_CLUSTERING       12

#define MOD_MPI_OFFSET                  0
#define MOD_MPIIO_OFFSET                80 
#define MOD_POSIXIO_OFFSET              140 
#define MOD_OMPTRACEPOINTS_OFFSET       200 
#define MOD_CUDA_OFFSET                 220
#define MOD_CUFFT_OFFSET                400
#define MOD_CUBLAS_OFFSET               420

#define MOD_MPI_RANGE                   (MOD_MPIIO_OFFSET-MOD_MPI_OFFSET)
#define MOD_MPIIO_RANGE                 (MOD_POSIXIO_OFFSET-MOD_MPIIO_OFFSET)
#define MOD_POSIXIO_RANGE               (MOD_OMPTRACEPOINTS_OFFSET-MOD_POSIXIO_OFFSET)
#define MOD_OMPTRACEPOINTS_RANGE        (MOD_CUDA_OFFSET-MOD_OMPTRACEPOINTS_OFFSET)
#define MOD_CUDA_RANGE                  (MOD_CUFFT_OFFSET-MOD_CUDA_OFFSET)
#define MOD_CUFFT_RANGE                 (MOD_CUBLAS_OFFSET-MOD_CUFFT_OFFSET)
#define MOD_CUBLAS_RANGE                180



#if (MOD_CUBLAS_OFFSET+MOD_CUBLAS_RANGE > MAXSIZE_CALLTABLE ) 
#error MAXSIZE_CALLTABLE not big enought to hold all events
#endif 


struct ipm_module;
struct region;

typedef int(*initfunc_t)(struct ipm_module* mod, int flags);
typedef int(*outputfunc_t)(struct ipm_module* mod, int flags);
typedef int(*finalizefunc_t)(struct ipm_module* mod, int flags);

/* Add something module-specific to the XML output.
   This function has to use the ipm_printf routine for output, 
   passing ptr as the first argument and keeping track of the 
   number of bytes written */
typedef int(*xmlfunc_t)(struct ipm_module* mod, void *ptr, struct region *reg);

/* Called upon region enter/exit */
typedef int(*regfunc_t)(struct ipm_module* mod, int op, struct region *reg);

typedef struct ipm_module
{
  char           *name;
  initfunc_t      init;
  outputfunc_t    output;
  finalizefunc_t  finalize;
  xmlfunc_t       xml;
  regfunc_t       regfunc;
  int             state;
  int             ct_offs;    /* range and offset in the */
  int             ct_range;   /* call table */
} ipm_mod_t;

void ipm_module_init(struct ipm_module *mod);

extern ipm_mod_t modules[MAXNUM_MODULES];


#endif /* IPM_MODULES_H_INCLUDED */
