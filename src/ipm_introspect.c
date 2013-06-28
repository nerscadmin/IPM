
#include <string.h>

#include "ipm.h"
#include "ipm_introspect.h"
#include "calltable.h"
#include "hashtable.h"

pia_regid_t pia_current_region()
{
  struct region* reg; 

  reg = ipm_rstackptr;
  if(reg) { 
    return reg->id;
  } else {
    return 0;
  }
}


pia_regid_t pia_child_region(pia_regid_t rid)
{
  struct region* reg; 

  reg = rstack_find_region_by_id(ipm_rstack, rid);
  
  if(reg && reg->child) {
    return reg->child->id;
  } else {
    return -1;
  }
}


pia_regid_t pia_parent_region(pia_regid_t rid)
{
  struct region* reg; 

  reg = rstack_find_region_by_id(ipm_rstack, rid);
  
  if(reg && reg->parent) {
    return reg->parent->id;
  } else {
    return -1;
  }
}


pia_regid_t pia_next_region(pia_regid_t rid)
{
  struct region* reg; 

  reg = rstack_find_region_by_id(ipm_rstack, rid);

  if(reg && reg->next) {
    return reg->next->id;
  } else {
    return -1;
  }
}

pia_regid_t pia_find_region_by_name(char *name)
{
  struct region* reg; 

  reg = rstack_find_region_by_name(ipm_rstack, name);

  if(reg) { 
    return reg->id;
  } else {
    return -1;
  }
}

pia_ret_t pia_get_region_data(pia_regdata_t *rdata, pia_regid_t rid)
{
  struct region* reg=0; 
  
  reg = rstack_find_region_by_id(ipm_rstack, rid);  
  if( !reg ) {
    return PIA_NOTFOUND;
  }

  rdata->id=rid;
  strncpy(rdata->name, reg->name, PIA_MAXLEN_LABEL);
  rdata->count=reg->nexecs;
  rdata->wtime=reg->wtime;
  rdata->mtime=reg->mtime;

  return PIA_OK;
}



pia_act_t pia_find_activity_by_name(char *name)
{
  int i;
  
  for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
    if( !strcmp(name, ipm_calltable[i].name) ) {
      return i; /* i will be >= 0 */
    }
  }
  
  return PIA_NOTFOUND;
}

pia_ret_t pia_init_activity_data(pia_actdata_t *adata)
{
  adata->ncalls=0;
  adata->tmin=1.0e15;
  adata->tmax=0.0;
  adata->tsum=0.0;
  
  return PIA_OK;
}


pia_ret_t pia_get_activity_data(pia_actdata_t *adata,
				pia_act_t act)
{
  int i;
  int bytes;
  int rank;
  
  for( i=0; i<MAXSIZE_HASH; i++ ) {
    if( ipm_htable[i].count==0 ) 
      continue;
    
    if( KEY_GET_ACTIVITY(ipm_htable[i].key)==act ) {
	  
      bytes = KEY_GET_BYTES(ipm_htable[i].key);
      rank  = KEY_GET_RANK(ipm_htable[i].key);
    
      if( ipm_htable[i].t_min < adata->tmin ) 
	adata->tmin = ipm_htable[i].t_min;
      if( ipm_htable[i].t_max > adata->tmax ) 
	adata->tmax = ipm_htable[i].t_max;
      
      adata->tsum+=ipm_htable[i].t_tot;
      adata->ncalls+=ipm_htable[i].count;
    }
  }

  return PIA_OK;
}
