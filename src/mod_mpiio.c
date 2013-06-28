
#include <mpi.h>
#include <stdio.h>

#include "ipm.h"
#include "hashtable.h"
#include "hashkey.h"
#include "GEN.calltable_mpiio.h"
#include "GEN.fproto.mpiio.h"
#include "mod_mpiio.h"

int mod_mpiio_xml(ipm_mod_t* mod, void *ptr, struct region *reg);
int mod_mpiio_region(ipm_mod_t* mod, int op, struct region *reg);

mpiiodata_t mpiiodata[MAXNUM_REGIONS];

int mod_mpiio_init(ipm_mod_t* mod, int flags)
{
  int i;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_mpiio_init;
  mod->output   = 0;
  mod->finalize = 0;
  mod->xml      = mod_mpiio_xml;
  mod->regfunc  = mod_mpiio_region;
  mod->name     = "MPIIO";
  mod->ct_offs  = MOD_MPIIO_OFFSET;
  mod->ct_range = MOD_MPIIO_RANGE;

  for( i=0; i<MAXNUM_REGIONS; i++ ) {
    mpiiodata[i].iotime=0.0;
    mpiiodata[i].iotime_e=0.0;
  }

  copy_mpiio_calltable();

  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}

int mod_mpiio_xml(ipm_mod_t* mod, void *ptr, struct region *reg) 
{
  struct region *tmp;
  ipm_hent_t stats;
  double time;
  int res=0;
  
  if( !reg ) {
    time = ipm_mpiiotime();
  }
  else {
    time = mpiiodata[reg->id].iotime;
    
    if( (reg->flags)&FLAG_PRINT_EXCLUSIVE ) {
      tmp = reg->child;
      while(tmp) {
	time -= mpiiodata[tmp->id].iotime;
	tmp = tmp->next;
      }
    }
  }
  
  res+=ipm_printf(ptr, 
		  "<module name=\"%s\" time=\"%.5e\" ></module>\n",
		  mod->name, time);
  
  return res;
}


int mod_mpiio_region(ipm_mod_t *mod, int op, struct region *reg)
{
  double time=0.0;
  if( !reg ) return 0;
  
  time = ipm_mpiiotime();
  
  switch(op) 
    {
    case -1: /* exit */
      mpiiodata[reg->id].iotime += (time - (mpiiodata[reg->id].iotime_e));
      break;
      
    case 1: /* enter */
      mpiiodata[reg->id].iotime_e=time;
      break;
  }

  return 0;
}
