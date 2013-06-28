
#ifndef MOD_KEYHIST_H_INCLUDED
#define MOD_KEYHIST_H_INCLUDED

#include "hashkey.h"
#include "ipm_modules.h" 


#define NODEFMT_CALL        (1)
#define NODEFMT_RANK        (1<<1)
#define NODEFMT_BYTES       (1<<2)
#define NODEFMT_CALLSITE    (1<<3)
#define NODEFMT_REGION      (1<<4)
#define NODEFMT_COUNT       (1<<5)



/* 
  hashtable entry for event transitions 
  
  skey  - key of the start event
  ekey  - key of the end   event
  count - number of transtitions
  t_tot - summed time
*/
typedef struct ipm_xhent 
{
  IPM_KEY_TYPE    skey;
  IPM_KEY_TYPE    ekey;
  IPM_COUNT_TYPE  count;
  double          t_tot;

#ifdef KEYHIST_FULL_TIMING
  /* to have a couple of actual timing values */
  double          time[200];
#endif /* KEYHIST_FULL_TIMING */
} ipm_xhent_t;


#define XHENT_CLEAR(xhent_)			\
  KEY_CLEAR(xhent_.skey);			\
  KEY_CLEAR(xhent_.ekey);			\
  xhent_.count=0;				\
  xhent_.t_tot=0.0;


#define KEYHIST_TRACE(file_, key_)				\
  {								\
  char buf[256];						\
  unsigned fmt;							\
								\
  fmt = NODEFMT_CALL | NODEFMT_RANK | NODEFMT_BYTES |		\
    NODEFMT_CALLSITE | NODEFMT_REGION;				\
								\
  keyhist_key_printf(buf, key_, fmt);				\
								\
  fprintf(file_, "%s\n", buf);					\
  								\
  }


/* the hashtable for event transitions */
extern ipm_xhent_t ipm_xhtable[MAXSIZE_XHASH];
extern int ipm_xhspace;

/* 
 * info required for connecting the dots...
 */
extern IPM_KEY_TYPE last_hkey;    /* hashkey of last event */
extern double       last_tstamp;  /* timestamp of last evt */


#define IPM_XHASH_HKEY_COLL(xhtable_,k1_,k2_,idx_,ncoll_)	\
  {								\
    unsigned tests=0;						\
    idx_ = KEYPAIR_FIRSTHASH(k1_,k2_,MAXSIZE_XHASH);		\
    								\
    while( !(KEY_EQUAL(xhtable_[idx_].skey,k1_)) ||			\
	   !(KEY_EQUAL(xhtable_[idx_].ekey,k2_)))			\
      {									\
	if( ipm_xhspace>0 &&						\
	    KEY_ISNULL(xhtable_[idx_].skey) &&				\
	    KEY_ISNULL(xhtable_[idx_].ekey) )				\
	  {								\
	    KEY_ASSIGN( xhtable_[idx_].skey, k1_);			\
	    KEY_ASSIGN( xhtable_[idx_].ekey, k2_);			\
	    xhtable_[idx_].t_tot=0.0;					\
	    xhtable_[idx_].count=0;					\
	    ipm_xhspace--;						\
	    break;							\
	  }								\
	else {								\
	  tests++;							\
	  if( tests<MAXSIZE_XHASH ) {					\
	    idx_ = KEYPAIR_REHASH(k1_,k2_,MAXSIZE_XHASH,idx_,ncoll_);	\
	  } else {							\
	    idx_ = -1;							\
	    break;							\
	  }								\
	}								\
      }									\
    ncoll_+=tests;							\
  }


#define IPM_XHASH_HKEY(xhtable_,k1_,k2_,idx_)			\
  {                                                             \
    unsigned ncoll=0;						\
    								\
    IPM_XHASH_HKEY_COLL(xhtable_,k1_,k2_,idx_,ncoll);		\
    								\
  }


void xhtable_init(ipm_xhent_t *table);

int mod_keyhist_init(ipm_mod_t* mod, int flags);
int mod_keyhist_output(ipm_mod_t* mod, int flags);

void write_eventgraph();

#endif /* MOD_KEYHIST_H_INCLUDED */
