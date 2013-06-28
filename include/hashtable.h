
#ifndef HASHTABLE_H_INCLUDED
#define HASHTABLE_H_INCLUDED

#include <stdio.h>
#include "ipm_types.h"
#include "ipm_sizes.h"
#include "hashkey.h"


typedef struct ipm_hent
{
  double t_min, t_max, t_tot;
  IPM_COUNT_TYPE count; 
  IPM_KEY_TYPE   key;
} ipm_hent_t;


#define HENT_CLEAR(hent_) \
  hent_.count=0;	  \
  hent_.t_min=0.0;	  \
  hent_.t_max=0.0;	  \
  hent_.t_tot=0.0;	  \
  KEY_CLEAR(hent_.key)	  
  


typedef struct scanspec
{
  IPM_KEY_TYPE lo;
  IPM_KEY_TYPE hi;
} scanspec_t;

typedef struct scanstat
{
  ipm_hent_t hent;
  double bytesum;
} scanstats_t;


extern ipm_hent_t ipm_htable[MAXSIZE_HASH];
extern int ipm_hspace;

extern IPM_KEY_TYPE last_hkey;



#define IPM_HASH_HKEY_COLL(htable_,key_,idx_,ncoll_)	\
  {							\
    unsigned tests=0;					\
    idx_ = KEY_FIRSTHASH(key_,MAXSIZE_HASH);			\
    								\
    while( !(KEY_EQUAL(htable_[idx_].key,key_))) {		\
      if( (ipm_hspace>0) && KEY_ISNULL(htable_[idx_].key) ) {	\
	KEY_ASSIGN( htable_[idx_].key, key_);			\
	htable_[idx_].count = 0;				\
	htable_[idx_].t_tot = 0.0;				\
	htable_[idx_].t_min = 1.0e15;				\
	htable_[idx_].t_max = 0.0;				\
	ipm_hspace--;						\
	break;							\
      }								\
      else {							\
	tests++;						\
	if( tests<MAXSIZE_HASH ) {				\
	  idx_ = KEY_REHASH(key_,MAXSIZE_HASH,idx_,ncoll_);	\
	} else {						\
	  idx_ = -1;						\
	  break;						\
	}							\
      }								\
    }								\
    ncoll_+=tests;						\
  }


#define IPM_HASH_HKEY(htable_,key_,idx_)			\
  {								\
    unsigned ncoll=0;						\
    								\
    IPM_HASH_HKEY_COLL(htable_,key_,idx_,ncoll);		\
								\
  }								


#define IPM_MAKE_KEY(key_,actv_,reg_,csite_,rank_,tid_,bytes_)	     \
  KEY_CLEAR(key_);						     \
  KEY_SET_ACTIVITY(key_,actv_);					     \
  KEY_SET_REGION(key_,reg_);					     \
  KEY_SET_CALLSITE(key_,csite_);				     \
  KEY_SET_RANK(key_,rank_);					     \
  KEY_SET_TID(key_,tid_);					     \
  KEY_SET_BYTES(key_,bytes_);
  

/* This code will map a 32 bit integer into the nearest power of two

 v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; 
 
   If we want logarithmic buckets 0,1,2,4,8,..Bytes we could mask size
   with the above. 0 is not a power of two but the function above maps 
   0 to 0. 
*/
#define IPM_MPI_KEY(key_,call_,rank_,size_,reg_,csite_) \
  KEY_CLEAR(key_);					\
  KEY_SET_ACTIVITY(key_,call_);				\
  KEY_SET_REGION(key_,reg_);				\
  KEY_SET_RANK(key_,rank_);				\
  KEY_SET_BYTES(key_,size_);				\
  KEY_SET_CALLSITE(key_,csite_);


#define IPM_HASHTABLE_ADD(idx_,t_)				\
  {								\
    if( idx_>= 0 && idx_ < MAXSIZE_HASH ) {			\
      ipm_htable[idx_].count++;					\
      ipm_htable[idx_].t_tot+=t_;				\
      if( t>ipm_htable[idx_].t_max ) ipm_htable[idx_].t_max=t_;	\
      if( t<ipm_htable[idx_].t_min ) ipm_htable[idx_].t_min=t_;	\
    }								\
  }


void htable_init(ipm_hent_t *table);
void htable_dump(FILE *f, ipm_hent_t *t, int hdr);

void htable_remap_callsites(ipm_hent_t *table, int *map, int max);

void htable_clear(ipm_hent_t *table, scanspec_t spec);

int htable_scan(const ipm_hent_t *table, scanstats_t *stats, 
		scanspec_t spec );

int htable_scan_multi(const ipm_hent_t *table, int nspec,
		      ipm_hent_t stats[], scanspec_t spec[] );

int htable_scan_full(const ipm_hent_t *table, 
		     ipm_hent_t stats[], scanspec_t spec );

int htable_scan_activity(const ipm_hent_t *table, scanstats_t *stats,
			 unsigned beg, unsigned end );


void scanspec_restrict_activity(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_restrict_region(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_restrict_callsite(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_restrict_rank(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_restrict_tid(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_restrict_bytes(scanspec_t *spec, unsigned beg, unsigned end );

void scanspec_null(scanspec_t *spec);

void scanspec_unrestrict_all(scanspec_t *spec);

void scanspec_unrestrict_activity(scanspec_t *spec);
void scanspec_unrestrict_region(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_unrestrict_callsite(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_unrestrict_rank(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_unrestrict_tid(scanspec_t *spec, unsigned beg, unsigned end );
void scanspec_unrestrict_bytes(scanspec_t *spec, unsigned beg, unsigned end );

#endif /* HASHTABLE_H_INCLUDED */ 
