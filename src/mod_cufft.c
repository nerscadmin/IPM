
#include "ipm.h"
#include "ipm_modules.h"
#include "perfdata.h"
#include "mod_cufft.h"
#include "GEN.calltable_cufft.h"
#include "hashtable.h"

double ipm_cuffttime();

int mod_cufft_xml(ipm_mod_t* mod, void *ptr, struct region *reg);
int mod_cufft_region(ipm_mod_t*mod, int op, struct region *reg);

int mod_cufft_init(ipm_mod_t* mod, int flags)
{
  int i;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_cufft_init;
  mod->output   = mod_cufft_output;
  mod->finalize = 0;
  mod->xml      = mod_cufft_xml;
  mod->regfunc  = mod_cufft_region;
  mod->name     = "CUFFT";
  mod->ct_offs  = MOD_CUFFT_OFFSET;
  mod->ct_range = MOD_CUFFT_RANGE;

  copy_cufft_calltable();

  for( i=0; i<MAXNUM_REGIONS; i++ ) {    
    task.cufftdata[i].time=0.0;
    task.cufftdata[i].time_e=0.0;
  }

  mod->state    = STATE_ACTIVE;

  return IPM_OK;
}

int mod_cufft_output(ipm_mod_t* mod, int flags)
{
  
}

int mod_cufft_xml(ipm_mod_t* mod, void *ptr, struct region *reg) 
{
  struct region *tmp;
  ipm_hent_t stats;
  double time;
  int res=0;
  
  if( !reg ) {
    time = ipm_cuffttime();
  }
  else {
    time = task.cufftdata[reg->id].time;

    if( (reg->flags)&FLAG_PRINT_EXCLUSIVE ) {
      tmp = reg->child;
      while(tmp) {
	time -= task.cufftdata[tmp->id].time;
	tmp = tmp->next;
      }
    }
  }

  res+=ipm_printf(ptr, 
		  "<module name=\"%s\" time=\"%.5e\" ></module>\n",
		  mod->name, time);
  
  return res;
}


int mod_cufft_region(ipm_mod_t *mod, int op, struct region *reg)
{
  double time;
  if( !reg ) return 0;

  time = ipm_cuffttime();

  switch(op) 
    {
    case -1: /* exit */
      task.cufftdata[reg->id].time += 
	(time - (task.cufftdata[reg->id].time_e));
      break;
      
    case 1: /* enter */
      task.cufftdata[reg->id].time_e=time;
      break;
  }
  return 0;
}


