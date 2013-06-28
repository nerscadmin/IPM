#ifndef MOD_CUDA_H_INCLUDED
#define MOD_CUDA_H_INCLUDED

#include "ipm_modules.h"
#include "cuda_runtime_api.h"
#include "cuda.h"

int mod_cuda_init(ipm_mod_t* mod, int flags);
int mod_cuda_output(ipm_mod_t* mod, int flags);

#define CUDA_MAXNUM_KERNELS 512
#define CUDA_MAXNUM_STREAMS 256


/* for the runtime API */
typedef struct 
{
  char *kernel;
  cudaEvent_t start;
  cudaEvent_t stop;
  cudaStream_t stream;
} rt_kerneltime_t;


/* for the driver API */
typedef struct 
{
  CUfunction kernel;
  CUevent    start;
  CUevent    stop;
  CUstream   stream;
} dr_kerneltime_t;


extern rt_kerneltime_t rt_kernels[CUDA_MAXNUM_KERNELS];
extern dr_kerneltime_t dr_kernels[CUDA_MAXNUM_KERNELS];

typedef struct
{
  int          id;
  cudaStream_t stream;
} rt_stream_t;

typedef struct
{
  int          id;
  CUstream     stream;
} dr_stream_t;

extern rt_stream_t rt_streams[CUDA_MAXNUM_STREAMS];
extern dr_stream_t dr_streams[CUDA_MAXNUM_STREAMS];


typedef struct cudadata
{
  double time;
  double time_e;
} cudadata_t;


typedef struct cudaptr
{
  void *ptr;
  char name[MAXSIZE_FILENAME];
} cudaptr_t;

extern cudaptr_t cudaptr[CUDA_MAXNUM_KERNELS];


#define IPM_CUDA_BYTES_NONE_C( bytes_ ) \
  bytes_=0;

#define IPM_CUDA_BYTES_COUNT_C( bytes_ ) \
  bytes_ = count;

#define IPM_CUDA_BYTES_SIZE_C( bytes_ ) \
  bytes_ = size;

#define IPM_CUDA_BYTES_EXTENT_C( bytes_ ) \
  bytes_ = (extent.width)*(extent.height)*(extent.depth);

#define IPM_CUDA_BYTES_WIDTH_HEIGHT_C( bytes_ ) \
  bytes_ = (width)*(height);

#define IPM_CUDA_KEY(key_,call_,rank_,size_,reg_,csite_)     \
  KEY_CLEAR(key_);                                           \
  KEY_SET_ACTIVITY(key_,call_);                              \
  KEY_SET_REGION(key_,reg_);                                 \
  KEY_SET_RANK(key_,rank_);                                  \
  KEY_SET_BYTES(key_,size_);                                 \
  KEY_SET_CALLSITE(key_,csite_);


#define IPM_CUDA_RT_LAUNCH_PRE(ptr_, stream_)				\
  {									\
    /* find available entry in kernel timing table */			\
    slot=-1;								\
    for( i=0; i<CUDA_MAXNUM_KERNELS; i++ ) {				\
      if( !(rt_kernels[i].kernel) ) {					\
	rt_kernels[i].kernel=((char*)ptr_);				\
	slot=i;								\
	break;								\
      }									\
    }									\
    if( slot>=0 ) {							\
      cudaEventCreate( &(rt_kernels[slot].start) );			\
      cudaEventCreate( &(rt_kernels[slot].stop) );			\
      rt_kernels[i].stream=stream_;					\
      cudaEventRecord( rt_kernels[slot].start, stream_ );		\
    }									\
  }

#define IPM_CUDA_RT_LAUNCH_POST(stream_ )				\
  if( slot>=0 ) {							\
    cudaEventRecord( rt_kernels[slot].stop, stream_ );			\
  }									


#define IPM_CUDA_DR_LAUNCH_PRE(func_, stream_)				\
  {									\
    /* find available slot in kernel timing table */			\
    slot=-1;								\
    for( i=0; i<CUDA_MAXNUM_KERNELS; i++ ) {				\
      if( !(dr_kernels[i].kernel) ) {					\
	dr_kernels[i].kernel=func_;					\
	slot=i;								\
	break;								\
      }									\
    }									\
    if( slot>=0 ) {							\
      cuEventCreate( &(dr_kernels[slot].start), 0 );			\
      cuEventCreate( &(dr_kernels[slot].stop), 0 );			\
      dr_kernels[i].stream=stream_;					\
      cuEventRecord( dr_kernels[slot].start, stream_ );			\
    }									\
  }

#define IPM_CUDA_DR_LAUNCH_POST(stream_)				\
  if( slot>=0 ) {							\
    cuEventRecord( dr_kernels[slot].stop, stream_ );			\
  }									



#define IPM_CUDA_HOST_IDLE(stream_)					\
  {									\
    double t1, t2, t3;							\
    int prevstate;							\
    									\
    IPM_TIMESTAMP(t1);							\
    prevstate=ipm_state;						\
    ipm_state=STATE_NOTACTIVE;						\
    cudaStreamSynchronize(stream_);					\
    ipm_state=prevstate;						\
    IPM_TIMESTAMP(t2);							\
    t3=t2-t1;								\
    regid=ipm_rstackptr->id;						\
    									\
    IPM_CUDA_KEY(key, CUDA_HOST_IDLE_ID_GLOBAL, 0, 0, regid, 0);	\
    IPM_HASH_HKEY(ipm_htable, key, idx);				\
    IPM_HASHTABLE_ADD(idx,t3);						\
  }
    

#define IPM_CUDA_ADDPTR(ptr_, name_)				\
  {								\
    int i;							\
    for( i=0; i<CUDA_MAXNUM_KERNELS; i++ ) {			\
      if( cudaptr[i].ptr==0 ) {					\
	cudaptr[i].ptr=ptr_;					\
	strncpy(cudaptr[i].name,name_,MAXSIZE_FILENAME);	\
	break;							\
      }								\
    }								\
  }
  
  

#endif /* MOD_CUDA_H_INCLUDED */

