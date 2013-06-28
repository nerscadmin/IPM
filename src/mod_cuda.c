
#include "ipm.h"
#include "ipm_modules.h"
#include "perfdata.h"
#include "mod_cuda.h"
#include "GEN.calltable_cuda.h"

#include "driver_types.h"
#include "hashtable.h"

rt_kerneltime_t rt_kernels[CUDA_MAXNUM_KERNELS];
dr_kerneltime_t dr_kernels[CUDA_MAXNUM_KERNELS];

rt_stream_t rt_streams[CUDA_MAXNUM_STREAMS];
dr_stream_t dr_streams[CUDA_MAXNUM_STREAMS];

int next_stream_id=0;

cudadata_t cudadata[MAXNUM_REGIONS];

cudaptr_t cudaptr[CUDA_MAXNUM_KERNELS];

int mod_cuda_xml(ipm_mod_t* mod, void *ptr, struct region *reg);
int mod_cuda_region(ipm_mod_t*mod, int op, struct region *reg);

double ipm_cudatime();

int mod_cuda_init(ipm_mod_t* mod, int flags)
{
  int i;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_cuda_init;
  mod->output   = mod_cuda_output;
  mod->finalize = 0;
  mod->xml      = mod_cuda_xml;
  mod->regfunc  = mod_cuda_region;
  mod->name     = "CUDA";
  mod->ct_offs  = MOD_CUDA_OFFSET;
  mod->ct_range = MOD_CUDA_RANGE;

  copy_cuda_calltable();

  for( i=0; i<MAXNUM_REGIONS; i++ ) {    
    cudadata[i].time=0.0;
    cudadata[i].time_e=0.0;
  }

  for( i=0; i<CUDA_MAXNUM_KERNELS; i++ ) {
    rt_kernels[i].kernel=0;
    dr_kernels[i].kernel=0;
  }

  for( i=0; i<CUDA_MAXNUM_STREAMS; i++ ) {
    rt_streams[i].id=-1;
    dr_streams[i].id=-1;
  }

  for( i=0; i<CUDA_MAXNUM_KERNELS; i++ ) {
    cudaptr[i].ptr=0;
    cudaptr[i].name[0]=0;
  }

  mod->state    = STATE_ACTIVE;

  return IPM_OK;
}

int mod_cuda_output(ipm_mod_t* mod, int flags)
{
}

void ipm_cuda_kerneltiming() 
{
  float duration;
  int i, j, idx, sid;
  IPM_KEY_TYPE key;
  double t;
  int oldstate;
  cudaError_t rt_ret;
  int dr_ret;

  oldstate=ipm_state;
  ipm_state=STATE_NOTACTIVE;
  
  for( i=0; i<CUDA_MAXNUM_KERNELS; i++ ) {
    if( !(rt_kernels[i].kernel) )
      continue;
    
    rt_ret = cudaEventQuery(rt_kernels[i].stop);
    if( rt_ret!=cudaSuccess ) {
      /*
	fprintf(stderr, "error: %s\n", cudaGetErrorString(rt_ret));
      */
      continue;
    }

    cudaEventElapsedTime( &duration, 
			  rt_kernels[i].start,
			  rt_kernels[i].stop );
    
    duration /= 1000.0;

    sid=-1;
    for( j=0; j<CUDA_MAXNUM_STREAMS; j++ ) {
      if( rt_streams[j].id < 0 ) {
	rt_streams[j].stream = rt_kernels[i].stream;
	rt_streams[j].id=next_stream_id++;
	sid=rt_streams[j].id;
	break;
      }
      if( rt_streams[j].stream == rt_kernels[i].stream ) {
	sid=rt_streams[j].id;
	break;
      }
    }

    /*
    fprintf(stderr, "*** This kernel execution in stream %d\n",
	    sid);
    */

    /* call, rank, size, reg, csite */
    IPM_CUDA_KEY(key, CUDA_EXEC00_ID_GLOBAL+sid, 0, 0, ipm_rstackptr->id, 0);
    
    KEY_SET_SELECT(key, IPM_RESOURCE_POINTER);
    KEY_SET_POINTER(key, rt_kernels[i].kernel);
    /* KEY_SET_DATATYPE(key, sid); */
    IPM_HASH_HKEY(ipm_htable, key, idx);
    IPM_HASHTABLE_ADD(idx,(double)duration);

    rt_kernels[i].kernel=0;
    cudaEventDestroy( rt_kernels[i].start );
    cudaEventDestroy( rt_kernels[i].stop );
  }								


  for( i=0; i<CUDA_MAXNUM_KERNELS; i++ ) {
    if( !(dr_kernels[i].kernel) )
      continue;

    dr_ret=cuEventQuery(dr_kernels[i].stop);
    if( dr_ret!=CUDA_SUCCESS ) {
      /*
      fprintf(stderr, "error: %d\n", dr_ret);
      */
      continue;
    }

    cuEventElapsedTime( &duration, 
			dr_kernels[i].start,
			dr_kernels[i].stop );
    

    duration /= 1000.0;

    sid=-1;
    for( j=0; j<CUDA_MAXNUM_STREAMS; j++ ) {
      if( dr_streams[j].id < 0 ) {
	dr_streams[j].stream = dr_kernels[i].stream;
	dr_streams[j].id=next_stream_id++;
	sid=dr_streams[j].id;
	break;
      }
      if( dr_streams[j].stream == dr_kernels[i].stream ) {
	sid=dr_streams[j].id;
	break;
      }
    }

    /*
    fprintf(stderr, "*** This kernel execution in stream %d\n",
	    sid);
    */

    /* call, rank, size, reg, csite */
    IPM_CUDA_KEY(key, CUDA_EXEC00_ID_GLOBAL+sid, 0, 0, ipm_rstackptr->id, 0);

    KEY_SET_SELECT(key, IPM_RESOURCE_POINTER);
    KEY_SET_POINTER(key, dr_kernels[i].kernel);
    /* KEY_SET_DATATYPE(key, sid); */
    IPM_HASH_HKEY(ipm_htable, key, idx);
    IPM_HASHTABLE_ADD(idx,(double)duration);

    dr_kernels[i].kernel=0;
    cuEventDestroy( dr_kernels[i].start );
    cuEventDestroy( dr_kernels[i].stop );
  }

  ipm_state=oldstate;
}


int mod_cuda_xml(ipm_mod_t* mod, void *ptr, struct region *reg) 
{
  struct region *tmp;
  ipm_hent_t stats;
  double time;
  int res=0;
  
  if( !reg ) {
    time = ipm_cudatime();
  }
  else {
    time = cudadata[reg->id].time;

    if( (reg->flags)&FLAG_PRINT_EXCLUSIVE ) {
      tmp = reg->child;
      while(tmp) {
	time -= cudadata[tmp->id].time;
	tmp = tmp->next;
      }
    }
  }

  res+=ipm_printf(ptr, 
		  "<module name=\"%s\" time=\"%.5e\" ></module>\n",
		  mod->name, time);
  
  return res;
  
}


int mod_cuda_region(ipm_mod_t *mod, int op, struct region *reg)
{
  double time;
  if( !reg ) return 0;

  time = ipm_cudatime();

  switch(op) 
    {
    case -1: /* exit */
      cudadata[reg->id].time += (time - (cudadata[reg->id].time_e));
      break;
      
    case 1: /* enter */
      cudadata[reg->id].time_e=time;
      break;
  }

  return 0;
}


