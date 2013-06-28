#ifndef MOD_POSIXIO_H_INCLUDED
#define MOD_POSIXIO_H_INCLUDED

#include "ipm_modules.h"

int mod_posixio_init(ipm_mod_t* mod, int flags);

typedef struct iodata
{
  double iotime;
  double iotime_e;
} iodata_t;


#define IPM_POSIXIO_BYTES_NONE_C( bytes_ ) \
  bytes_=0;

#define IPM_POSIXIO_BYTES_COUNT_C( bytes_ ) \
  bytes_=count;

#define IPM_POSIXIO_BYTES_RETURN_COUNT_C( bytes_ ) \
  (rv!=-1)?(bytes_=rv):(bytes_=0)

#define IPM_POSIXIO_BYTES_NMEMB_C( bytes_ ) \
  bytes_=nmemb*size;

#define IPM_POSIXIO_BYTES_RETURN_NMEMB_C( bytes_ ) \
  bytes_=rv*size;

#define IPM_POSIXIO_BYTES_RETURN_EOF_C( bytes_ ) \
  (rv==EOF)?(bytes_=0):(bytes_=1);

#define IPM_POSIXIO_BYTES_CHAR_C( bytes_ ) \
  bytes_=sizeof(char);


#define IPM_POSIXIO_KEY(key_,call_,rank_,size_,reg_,csite_) \
  KEY_CLEAR(key_);					     \
  KEY_SET_ACTIVITY(key_,call_);				     \
  KEY_SET_REGION(key_,reg_);				     \
  KEY_SET_RANK(key_,rank_);				     \
  KEY_SET_BYTES(key_,size_);				     \
  KEY_SET_CALLSITE(key_,csite_);

#endif /* MOD_POSIXIO_H_INCLUDED */
