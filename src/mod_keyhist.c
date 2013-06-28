
#include <string.h>
#include <mpi.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_modules.h"
#include "perfdata.h"
#include "hashkey.h"
#include "hashtable.h"
#include "calltable.h"
#include "mod_keyhist.h"
#include "GEN.calltable_mpi.h"


/* --- globals -- */

IPM_KEY_TYPE  last_hkey;
double        last_tstamp;
ipm_xhent_t   ipm_xhtable[MAXSIZE_XHASH];
int           ipm_xhspace;


void xhtable_init(ipm_xhent_t *table) {
  int i;
  for(i=0; i<MAXSIZE_XHASH; i++ ) {
    XHENT_CLEAR(table[i]);
  }
}

void xhtable_remap_callsites(ipm_xhent_t *table, int *map, int maxid)
{
  int i; int cs;
  
  for( i=0; i<MAXSIZE_XHASH; i++ ) {
    if( table[i].count==0 )
      continue;
    
    cs=KEY_GET_CALLSITE(table[i].skey);
    if( 0<=cs && cs<=maxid && map[cs] && cs!=map[cs] ) {
      KEY_SET_CALLSITE(table[i].skey,map[cs]);
    }

    cs=KEY_GET_CALLSITE(table[i].ekey);
    if( 0<=cs && cs<=maxid && map[cs] && cs!=map[cs] ) {
      KEY_SET_CALLSITE(table[i].ekey,map[cs]);
    }
  }
}


int mod_keyhist_init(ipm_mod_t *mod, int flags) 
{
  mod->state    = STATE_IN_INIT;
  mod->init     = mod_keyhist_init;
  mod->output   = mod_keyhist_output;
  mod->finalize = 0; 
  mod->name     = "KEYHIST";

  KEY_CLEAR(last_hkey);
  xhtable_init(ipm_xhtable);
  ipm_xhspace = MAXSIZE_XHASH;
  
  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}




#define GET_ACTIVITY_NAME(key_,name_)					\
  {									\
    int actv = KEY_GET_ACTIVITY(key_);					\
    if( 0<=actv && actv<MAXSIZE_CALLTABLE )				\
      name_ = ipm_calltable[actv].name;					\
    else								\
      name_ = "(outofrange)";						\
    if( !name_ ) name_ = "(unset)";					\
  }

void keyhist_key_printf(char *buf, IPM_KEY_TYPE key, unsigned fmt);

int mod_keyhist_output(ipm_mod_t *mod, int flags) {
  char buf1[256];
  char buf2[256];
  char fname[256];
  unsigned fmt;
  FILE *f;
  int i, j;

  fmt = NODEFMT_COUNT | NODEFMT_CALL | NODEFMT_RANK | 
    NODEFMT_BYTES | NODEFMT_CALLSITE | NODEFMT_REGION;

  sprintf(fname, "%s.keyhist.%d.txt", task.fname, task.taskid);
  f=fopen(fname, "w");

  for( i=0; i<MAXSIZE_XHASH; i++ ) {
    if( KEY_ISNULL(ipm_xhtable[i].skey)&&
	KEY_ISNULL(ipm_xhtable[i].ekey) ) 
      continue;
    
    keyhist_key_printf(buf1, (ipm_xhtable[i].skey), fmt);
    keyhist_key_printf(buf2, (ipm_xhtable[i].ekey), fmt);
    
    fprintf(f, "%s -- %"IPM_COUNT_TYPEF" --> %s",
	    buf1, ipm_xhtable[i].count, buf2);

    fprintf(f, " (%4.2f) ", ipm_xhtable[i].t_tot);

#ifdef KEYHIST_FULL_TIMING
    for( j=0; j<ipm_xhtable[i].count; j++ ) {
      fprintf(f, " %5.3f ", ipm_xhtable[i].time[j]);
    }
#endif /* KEYHIST_FULL_TIMING */

    fprintf(f, "\n");
  }
  
  fclose(f);
}


void keyhist_key_printf(char *buf, IPM_KEY_TYPE key, unsigned fmt)
{
  char str[256];
  int rank, drank;
  int call, bytes;
  int csite, reg, idx;
  IPM_COUNT_TYPE count;
  char *name;

  rank = KEY_GET_RANK(key);
  call = KEY_GET_ACTIVITY(key);
  bytes = KEY_GET_BYTES(key);
  csite = KEY_GET_CALLSITE(key);
  reg = KEY_GET_REGION(key);

  IPM_HASH_HKEY(ipm_htable, key, idx);
  count = ipm_htable[idx].count;

  if( call<0 || call>=MAXSIZE_CALLTABLE ) 
    return;

  sprintf(buf, "[ ");

  if( (fmt & NODEFMT_COUNT) ) {
    sprintf(str, "%"IPM_COUNT_TYPEF"x ", count);
    strcat(buf, str);
  }

  if( fmt & NODEFMT_CALL ) {
    GET_ACTIVITY_NAME(key,name);
    sprintf(str, "%s", name);
    strcat(buf, str);
  }


  if( (fmt & NODEFMT_RANK) ) {
    switch( rank ) {
    case IPM_RANK_NULL:
      /* sprintf( str, "/(NULL)"); */
      sprintf( str, "");
      break;
    case IPM_RANK_ANY_SOURCE:
      sprintf( str, "/(ANY)");
      break;
    case IPM_RANK_ALL:
      /* sprintf( str, "/(ALL)"); */
      sprintf( str, "");
      break;
    
    default:
      drank = rank-(task.taskid);
      sprintf(str, "/%s%d", (drank>=0)?"+":"", drank);
      if( (ipm_calltable[call].attr)&DATA_RX || 
	  (ipm_calltable[call].attr)&DATA_TX || 
	  (ipm_calltable[call].attr)&DATA_TXRX 
#ifdef HAVE_MPI
	  || call==MPI_WAIT_ID_GLOBAL 
	  || call==MPI_WAITALL_ID_GLOBAL 
	  || call==MPI_WAITANY_ID_GLOBAL 
#endif /* HAVE_MPI */
	  ) {
	drank = rank-task.taskid;
	sprintf(str, "/%s%d", (drank>=0)?"+":"", drank);
      }
      else 
	sprintf(str, "/%d", rank);
    }

    strcat(buf, str);
  }
  
  if( (fmt & NODEFMT_BYTES) && 
      ( !(ipm_calltable[call].attr&DATA_NONE) 
#ifdef HAVE_MPI
	|| call==MPI_WAIT_ID_GLOBAL 
	|| call==MPI_WAITALL_ID_GLOBAL 
	|| call==MPI_WAITANY_ID_GLOBAL 
#endif
	)) 
    { 
      sprintf(str, " %uB", bytes);
      strcat(buf, str);
    }

  if( (fmt & NODEFMT_CALLSITE) )
    {
      sprintf(str, " cs=%u", csite);
      strcat(buf, str);
    }
  
  if( (fmt & NODEFMT_REGION) )
    {
      sprintf(str, " r=%u", reg);
      strcat(buf, str);
    }
  strcat(buf, " ]");
}

