
#include "ipm.h"
#include "ipm_modules.h"
#include "perfdata.h"
#include "mod_cublas.h"
#include "GEN.calltable_cublas.h"
#include "hashtable.h"

double ipm_cublastime();

int mod_cublas_xml(ipm_mod_t* mod, void *ptr, struct region *reg);
int mod_cublas_region(ipm_mod_t*mod, int op, struct region *reg);

int mod_cublas_init(ipm_mod_t* mod, int flags)
{
  int i;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_cublas_init;
  mod->output   = mod_cublas_output;
  mod->finalize = 0;
  mod->xml      = mod_cublas_xml;
  mod->regfunc  = mod_cublas_region;
  mod->name     = "CUBLAS";
  mod->ct_offs  = MOD_CUBLAS_OFFSET;
  mod->ct_range = MOD_CUBLAS_RANGE;

  copy_cublas_calltable();
  
  for( i=0; i<MAXNUM_REGIONS; i++ ) {    
    task.cublasdata[i].time=0.0;
    task.cublasdata[i].time_e=0.0;
  }
  
  mod->state    = STATE_ACTIVE;

  return IPM_OK;
}

int mod_cublas_output(ipm_mod_t* mod, int flags)
{
  
}

int mod_cublas_xml(ipm_mod_t* mod, void *ptr, struct region *reg) 
{
  struct region *tmp;
  ipm_hent_t stats;
  double time;
  int res=0;
  
  if( !reg ) {
    time = ipm_cublastime();
  }
  else {
    time = task.cublasdata[reg->id].time;

    if( (reg->flags)&FLAG_PRINT_EXCLUSIVE ) {
      tmp = reg->child;
      while(tmp) {
	time -= task.cublasdata[tmp->id].time;
	tmp = tmp->next;
      }
    }
  }

  res+=ipm_printf(ptr, 
		  "<module name=\"%s\" time=\"%.5e\" ></module>\n",
		  mod->name, time);
  
  return res;
}


int mod_cublas_region(ipm_mod_t *mod, int op, struct region *reg)
{
  double time;
  if( !reg ) return 0;

  time = ipm_cublastime();

  switch(op) 
    {
    case -1: /* exit */
      task.cublasdata[reg->id].time += 
	(time - (task.cublasdata[reg->id].time_e));
      break;
      
    case 1: /* enter */
      task.cublasdata[reg->id].time_e=time;
      break;
  }
  return 0;
}


