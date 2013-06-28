#ifndef IPM_DEBUG_H_INCLUDED
#define IPM_DEBUG_H_INCLUDED

#include "perfdata.h"

/*
 * IPMDBG macro
 */


#ifdef HAVE_MPI
/* print with rank ID */
#define PRINTF_DBG(format_,args_...) {					\
    fprintf(stderr, "IPM%3d: "format_, task.taskid, ## args_ ); }
#else
/* print with pid */
#define PRINTF_DBG(format_,args_...) {				\
    fprintf(stderr, "IPM%6ld: "format_, ((long)task.pid), ## args_ ); }
#endif /* HAVE_MPI */


#ifdef DEBUG

#define IPMDBG(format_,args_...) PRINTF_DBG(format_, ## args_)
#define IPMDBG0(format_,args_...) { if(task.taskid==0) { PRINTF_DBG(format_, ## args_); } }

#else 

#define IPMDBG(format_,args_...) 				\
  if( (task.flags)&FLAG_DEBUG ) { PRINTF_DBG(format_, ## args_); } 

#define IPMDBG0(format_,args_...) 				\
  if( (task.flags)&FLAG_DEBUG && (task.taskid==0) ) { PRINTF_DBG(format_, ## args_); } 


#endif /* DEBUG */



/* 
 * IPMERR macro 
 */

#ifdef HAVE_MPI 
#define IPMERR(format_,args_...) { \
    fprintf(stderr, "IPM%3d: ERROR "format_, task.taskid, ## args_ ); }
#else
#define IPMERR(format_,args_...) { \
    fprintf(stderr, "IPM%6ld: ERROR "format_, ((long)task.pid), ## args_ ); }
#endif /* HAVE_MPI */


#ifdef HAVE_MPI 
#define IPMMSG(format_,args_...) { \
    fprintf(stderr, "IPM%3d: "format_, task.taskid, ## args_ ); }
#else
#define IPMMSG(format_,args_...) { \
    fprintf(stderr, "IPM%6ld: "format_, ((long)task.pid), ## args_ ); }
#endif /* HAVE_MPI */


#endif /* IPM_DEBUG_H_INCLUDED */
