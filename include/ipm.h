#ifndef IPM_H_INCLUDED
#define IPM_H_INCLUDED

#define IPM_VERSION "2.0.2"

#include "config.h"
#include "perfdata.h"
#include "ipm_debug.h"

/*
 * IPM return/error values 
 */
#define IPM_OK           0
#define IPM_EOTHER       1 /* unspecified error */
#define IPM_ENOMEM       2 /* insufficient memory */
#define IPM_EINVAL       3 /* invalid argument(s) */
#define IPM_ESYS         4 /* system call failed */

/*
 * delay MPI_Finalize() until IPM wraps up 
 */ 

/* delaying MPI_Finalize seems to cause problems on 
   parastation MPI; therefore below introduces a build-
   option to disable delaying */

#ifndef HAVE_NO_DELAYED_FINALIZE
#define DELAYED_MPI_FINALIZE 1 
#endif 

#endif /* IPM_H_INCLUDED */
