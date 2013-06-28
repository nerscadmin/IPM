

#include <mpi.h>
#include <stdio.h>
#include <stdarg.h>

#include <string.h>

#include "perfdata.h"
#include "regstack.h"
#include "ipm_sizes.h"
#include "GEN.fproto.mpi.h"

#define CHECK_REGION(reg_, ptr_)		\
  {						\
    int len=0;					\
    reg_=(char*)(ptr_);				\
    len = strlen(reg_);				\
    if( len==0 || len>MAXSIZE_REGLABEL ) {	\
      reg_=0;					\
    }						\
  }


int ipm_control(const int ctl, char *cmd, void *data) 
{
  char *reg;
  
  switch( ctl ) 
    {
    case 1:   /* enter region, IPM1 compatible */
      CHECK_REGION(reg, cmd);
      if(reg) 
	ipm_region(1, reg);
      break;
      
    case -1:  /* exit region, IPM1 compatible */
      if(reg) 
	CHECK_REGION(reg, cmd);
      ipm_region(-1, reg);
      break;
      
      /* general case */
    case 0:
    default:
      if( !strncmp(cmd, "enter", 5) ) {
	CHECK_REGION(reg, data);
	if(reg)
	  ipm_region(1, reg);
      }
      
      if( !strncmp(cmd, "exit", 4) ) {
	CHECK_REGION(reg, data);
	if(reg)
	  ipm_region(-1, reg);
      }  

#ifdef HAVE_POSIXIO_TRACE
      if( !strncmp(cmd, "traceoff", 8) )
	task.tracestate = 0;
      if( !strncmp(cmd, "traceon", 7) )
	task.tracestate = 1;
#endif  
    }
  
  return 0;
}


int MPI_Pcontrol(const int ctl,...) 
{
  int res;
  va_list ap;
  char *cmd;
  void *data;

  /* ignore MPI_Pcontrol calls if not initialized */
  if( ipm_state==STATE_NOTINIT ) 
    return 0;
  
  va_start(ap, ctl);
  cmd  = va_arg(ap, char *);
  data = va_arg(ap, void *);
  va_end(ap);  

  res = ipm_control(ctl, cmd, data);
  return res;
}


void MPI_PCONTROL_F(const int *ctl, char *cmd, char *data) 
{
  int myctl;

  /* ignore MPI_Pcontrol calls if not initialized */
  if( ipm_state==STATE_NOTINIT ) 
    return;

  myctl = (ctl?(*ctl):0);
  ipm_control(myctl, cmd, data);

  return;
}
