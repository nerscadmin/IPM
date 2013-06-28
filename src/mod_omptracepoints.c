
#ifdef HAVE_OMPTRACEPOINTS

#include <stdio.h>

#include "ipm.h"
#include "ipm_modules.h"
#include "mod_omptracepoints.h"
#include "calltable.h"
#include "hashtable.h"

int mod_omptracepoints_xml(ipm_mod_t* mod, void *ptr, struct region *reg);
int mod_omptracepoints_region(ipm_mod_t*mod, int op, struct region *reg);

/* --- global variables --- */
ompstats_t ompstats[MAXNUM_THREADS];
int nthreads, maxthreads=1;

extern int num_levels;


int mod_omptracepoints_init(ipm_mod_t *mod, int flags)
{
  int i;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_omptracepoints_init;
  mod->output   = 0;
  mod->finalize = 0; 
  mod->xml      = mod_omptracepoints_xml;
  mod->name     = "OMPTRACEPOINTS";
  mod->ct_offs  = MOD_OMPTRACEPOINTS_OFFSET;
  mod->ct_range = MOD_OMPTRACEPOINTS_RANGE;

  for( i=0; i<MAXNUM_THREADS; i++ ) {
    ompstats[i].twork=0.0;
    ompstats[i].tidle=0.0;
    ompstats[i].nenter=0;
  }

  for( i=0; i<MAXNUM_REGIONS; i++ ) {
    task.ompdata[i].omptime    = 0.0;
    task.ompdata[i].omptime_e  = 0.0;
    task.ompdata[i].idletime   = 0.0;
    task.ompdata[i].idletime_e = 0.0;
  }

  ipm_calltable[OMP_PARALLEL_ID_GLOBAL].name = 
    "@OMP_PARALLEL";
  ipm_calltable[OMP_PARALLEL_ID_GLOBAL].attr = 0;

  ipm_calltable[OMP_IDLE_ID_GLOBAL].name = 
    "@OMP_IDLE";
  ipm_calltable[OMP_IDLE_ID_GLOBAL].attr = 0;

  num_levels = 0;

  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}

int mod_omptracepoints_xml(ipm_mod_t* mod, void *ptr, struct region *reg) 
{
  struct region *tmp;
  ipm_hent_t stats;
  double time, idle;
  int res=0;
  
  if( !reg ) {
    time = ipm_omptime();
    idle = ipm_ompidletime();
  }
  else { 
    time = task.ompdata[reg->id].omptime;
    idle = task.ompdata[reg->id].idletime;

    if( (reg->flags)&FLAG_PRINT_EXCLUSIVE ) {
      tmp = reg->child;
      while(tmp) {
	time -= task.ompdata[tmp->id].omptime;
	idle -= task.ompdata[tmp->id].idletime;
	tmp = tmp->next;
      }
    }
  }
  
  res+=ipm_printf(ptr, 
		  "<module name=\"%s\" time=\"%.5e\" idletime=\"%.5e\" maxthreads=\"%d\" ></module>\n",
		  mod->name, time, idle, maxthreads);
  
  return res;
  
}

int mod_omptracepoints_region(ipm_mod_t *mod, int op, struct region *reg)
{
  double omptime;
  double idletime;

  if( !reg ) return 0;

  omptime  = ipm_omptime();
  idletime = ipm_ompidletime();

  switch(op) 
    {
    case -1:  /* exit */
      task.ompdata[reg->id].omptime  += 
	(omptime  - (task.ompdata[reg->id].omptime_e));
      task.ompdata[reg->id].idletime += 
	(idletime - (task.ompdata[reg->id].idletime_e));
      break;
      
    case 1:  /* enter */
      task.ompdata[reg->id].omptime_e  = omptime;
      task.ompdata[reg->id].idletime_e = idletime;
      break;
  }

  return 0;
}



/* wrappers for cray */
#undef TRACEPOINTS_PGI
#define TRACEPOINTS_CRAY
#include "mod_omptracepoints_wrap.h"

/* wrappers for PGI */
#define TRACEPOINTS_PGI
#undef TRACEPOINTS_CRAY
#include "mod_omptracepoints_wrap.h"

#endif /* HAVE_OMPTRACEPOINTS */
