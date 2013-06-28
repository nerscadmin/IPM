#ifndef MOD_CUFFT_H_INCLUDED
#define MOD_CUFFT_H_INCLUDED

#include "ipm_modules.h"
#include "cuda.h"
#include "cufft.h"

int mod_cufft_init(ipm_mod_t* mod, int flags);
int mod_cufft_output(ipm_mod_t* mod, int flags);

#define IPM_CUFFT_KEY(key_,call_,rank_,size_,reg_,csite_)     \
  KEY_CLEAR(key_);					      \
  KEY_SET_ACTIVITY(key_,call_);				      \
  KEY_SET_REGION(key_,reg_);				      \
  KEY_SET_RANK(key_,rank_);				      \
  KEY_SET_BYTES(key_,size_);				      \
  KEY_SET_CALLSITE(key_,csite_);


#define IPM_CUFFT_BYTES_NONE_C(bytes_)   bytes_=0;
#define IPM_CUFFT_BYTES_NX_C(bytes_)     bytes_=nx;
#define IPM_CUFFT_BYTES_NXNY_C(bytes_)   bytes_=nx*ny;
#define IPM_CUFFT_BYTES_NXNYNZ_C(bytes_) bytes_=nx*ny*nz;


typedef struct cufftdata
{
  double time;
  double time_e;
} cufftdata_t;


#endif /* MOD_CUFFT_H_INCLUDED */

