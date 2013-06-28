#ifndef MOD_CUBLAS_H_INCLUDED
#define MOD_CUBLAS_H_INCLUDED

#include "ipm_modules.h"
#include "cuda.h"
#include "cublas.h"

int mod_cublas_init(ipm_mod_t* mod, int flags);
int mod_cublas_output(ipm_mod_t* mod, int flags);

#define IPM_CUFFT_KEY(key_,call_,rank_,size_,reg_,csite_)     \
  KEY_CLEAR(key_);					      \
  KEY_SET_ACTIVITY(key_,call_);				      \
  KEY_SET_REGION(key_,reg_);				      \
  KEY_SET_RANK(key_,rank_);				      \
  KEY_SET_BYTES(key_,size_);				      \
  KEY_SET_CALLSITE(key_,csite_);

#define IPM_CUBLAS_BYTES_NONE_C(bytes_)        bytes_=0;
#define IPM_CUBLAS_BYTES_MNK_C(bytes_)         bytes_=m*n*k;
#define IPM_CUBLAS_BYTES_NELEMSIZE_C(bytes_)   bytes_=n*elemSize;


typedef struct cublasdata
{
  double time;
  double time_e;
} cublasdata_t;

#endif /* MOD_CUFFT_H_INCLUDED */

