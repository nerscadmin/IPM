

#include "ipm.h"
#include "perfdata.h"
#include "mod_posixio.h"
#include "ipm_modules.h"
#include "hashtable.h"
#include "GEN.calltable_posixio.h"

//iodata_t iodata[MAXNUM_REGIONS];

int mod_posixio_xml(ipm_mod_t* mod, void *ptr, struct region *reg);
int mod_posixio_region(ipm_mod_t *mod, int op, struct region *reg); 

int mod_posixio_init(ipm_mod_t* mod, int flags)
{
  char fname[256];
  int i, id;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_posixio_init;
  mod->output   = 0;
  mod->finalize = 0;
  mod->xml      = mod_posixio_xml;
  mod->regfunc  = mod_posixio_region;
  mod->name     = "POSIXIO";
  mod->ct_offs  = MOD_POSIXIO_OFFSET;
  mod->ct_range = MOD_POSIXIO_RANGE;

  copy_posixio_calltable();
  id=task.pid;

  for(i=0; i<MAXNUM_REGIONS; i++ ) {
    task.iodata[i].iotime=0.0;
    task.iodata[i].iotime_e=0.0;
  }

#if defined(HAVE_POSIXIO_TRACE) && !defined(HAVE_MPI) 
  if( !task.tracefile ) {
    int i;

    sprintf(fname, "%s.trace.%d.txt", task.fname, id);
    task.tracestate=0;

    i=1;
    while( !access(fname, F_OK) ) 
      {
	sprintf(fname, "%s.trace.%d-%d.txt", task.fname, id, i);
	i++;
      }
    task.tracefile=fopen(fname, "w");
    task.tracestate=1;
  }
#endif

  mod->state    = STATE_ACTIVE;

  return IPM_OK;
}

int mod_posixio_xml(ipm_mod_t* mod, void *ptr, struct region *reg) 
{
  struct region *tmp;
  ipm_hent_t stats;
  double time;
  int res=0;
  
  if( !reg ) {
    time = ipm_iotime();
  } else {
    time = task.iodata[reg->id].iotime;
    
    if( (reg->flags)&FLAG_PRINT_EXCLUSIVE ) {
      tmp = reg->child;
      while(tmp) {
	time -= task.iodata[tmp->id].iotime;
	tmp = tmp->next;
      }
    }
  }
   
  res+=ipm_printf(ptr, 
		  "<module name=\"%s\" time=\"%.5e\" ></module>\n",
		  mod->name,  time);
  
  return res;
  
}


int mod_posixio_region(ipm_mod_t *mod, int op, struct region *reg)
{
  double time;
  if( !reg ) return 0;

  time = ipm_iotime();

  switch(op) 
    {
    case -1: /* exit */
      task.iodata[reg->id].iotime += (time - (task.iodata[reg->id].iotime_e));
      break;
      
    case 1: /* enter */
      task.iodata[reg->id].iotime_e=time;
      break;
  }

  return 0;
}
