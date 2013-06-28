#ifndef HASHKEY_H_INCLUDED
#define HASHKEY_H_INCLUDED

#include <stdio.h>

#include "ipm_types.h"
#include "ipm_sizes.h"

/*

 k1 is the context key, k2 is the resource key
 k1.select defines which one of several possible resource keys it is

     0123456789012345 0123456789012345 0123456789012345 0123456789012345 
 k1: aaaaaaaaaarrrrrr rrrrrrrrtttttttt cccccccccccccccc ddddddddoooossss
 k2: bbbbbbbbbbbbbbbb bbbbbbbbbbbbbbbb uupppppppppppppp pppppppppppppppp (select==0)
 k2: xxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxx (select==1)

     component          #bits   range
 -------------------------------------
  a: activity/call ID     10     1024
  r: region ID            14    16384
  t: thread ID             8      256
  c: callsite ID          16      64k
  d: datatype              8      256
  o: operation             4       16
  s: select resource       4       16
  b: buffer size          32      4.2 bn/GB
  p: partner ID           30      1.0 bn
  x: 64-bit pointer value
  u: user/undef/rsvd

*/


#ifdef HAVE_UINT128
/* native 128-bit integer implementation goes here, 
   if not available, we implement it as a pair of 64 bit ints */

#else
/* implement the 128 bit key as a pair of 64 bit keys  */
 
typedef struct {
  unsigned long long k1;
  unsigned long long k2;
} ipm_key_t;

#define IPM_KEY_TYPE  ipm_key_t
#define IPM_KEY_TYPEF "%016llX%016llX"


#define KEY_MASK_ACTIVITY   (0xFFC0000000000000ULL) /* offs: 54 */
#define KEY_MASK_REGION     (0x003FFF0000000000ULL) /* offs: 40 */
#define KEY_MASK_TID        (0x000000FF00000000ULL) /* offs: 32 */
#define KEY_MASK_CALLSITE   (0x00000000FFFF0000ULL) /* offs: 16 */
#define KEY_MASK_DATATYPE   (0x000000000000FF00ULL) /* offs:  8 */
#define KEY_MASK_OPERATION  (0x00000000000000F0ULL) /* offs:  4 */
#define KEY_MASK_SELECT     (0x000000000000000FULL) /* offs:  0 */
#define KEY_MASK_BYTES      (0xFFFFFFFF00000000ULL) /* offs: 32 */
#define KEY_MASK_RANK       (0x000000003FFFFFFFULL) /* offs:  0 */
#define KEY_MASK_POINTER    (0xFFFFFFFFFFFFFFFFULL) /* offs:  0 */


#define KEY_OFFS_ACTIVITY    54
#define KEY_OFFS_REGION      40
#define KEY_OFFS_TID         32
#define KEY_OFFS_CALLSITE    16
#define KEY_OFFS_DATATYPE     8
#define KEY_OFFS_OPERATION    4
#define KEY_OFFS_SELECT       0
#define KEY_OFFS_BYTES       32
#define KEY_OFFS_RANK         0
#define KEY_OFFS_POINTER      0


#define KEY_MAX_ACTIVITY   (KEY_MASK_ACTIVITY  >> KEY_OFFS_ACTIVITY)  
#define KEY_MAX_REGION     (KEY_MASK_REGION    >> KEY_OFFS_REGION)
#define KEY_MAX_TID        (KEY_MASK_TID       >> KEY_OFFS_TID)
#define KEY_MAX_CALLSITE   (KEY_MASK_CALLSITE  >> KEY_OFFS_CALLSITE)
#define KEY_MAX_DATATYPE   (KEY_MASK_DATATYPE  >> KEY_OFFS_DATATYPE)
#define KEY_MAX_OPERATION  (KEY_MASK_OPERATION >> KEY_OFFS_OPERATION)
#define KEY_MAX_SELECT     (KEY_MASK_SELECT    >> KEY_OFFS_SELECT)
#define KEY_MAX_BYTES      (KEY_MASK_BYTES     >> KEY_OFFS_BYTES)
#define KEY_MAX_RANK       (KEY_MASK_RANK      >> KEY_OFFS_RANK)
#define KEY_MAX_POINTER    (KEY_MASK_POINTER   >> KEY_OFFS_POINTER)


#define KEY_MIN_ACTIVITY   0x0
#define KEY_MIN_REGION     0x0
#define KEY_MIN_TID        0x0
#define KEY_MIN_CALLSITE   0x0
#define KEY_MIN_DATATYPE   0x0
#define KEY_MIN_OPERATION  0x0
#define KEY_MIN_SELECT     0x0
#define KEY_MIN_BYTES      0x0
#define KEY_MIN_RANK       0x0
#define KEY_MIN_POINTER    0x0


#define KEY_GET_ACTIVITY(k_)    (((k_.k1)&KEY_MASK_ACTIVITY) >> KEY_OFFS_ACTIVITY)
#define KEY_GET_REGION(k_)      (((k_.k1)&KEY_MASK_REGION)   >> KEY_OFFS_REGION)
#define KEY_GET_TID(k_)         (((k_.k1)&KEY_MASK_TID)      >> KEY_OFFS_TID)
#define KEY_GET_CALLSITE(k_)    (((k_.k1)&KEY_MASK_CALLSITE) >> KEY_OFFS_CALLSITE)
#define KEY_GET_DATATYPE(k_)    (((k_.k1)&KEY_MASK_DATATYPE) >> KEY_OFFS_DATATYPE)
#define KEY_GET_OPERATION(k_)   (((k_.k1)&KEY_MASK_OPERATION) >> KEY_OFFS_OPERATION)
#define KEY_GET_SELECT(k_)      (((k_.k1)&KEY_MASK_SELECT) >> KEY_OFFS_SELECT)
#define KEY_GET_BYTES(k_)       (((k_.k2)&KEY_MASK_BYTES )   >> KEY_OFFS_BYTES)
#define KEY_GET_RANK(k_)        (((k_.k2)&KEY_MASK_RANK)     >> KEY_OFFS_RANK)
#define KEY_GET_POINTER(k_)     (((k_.k2)&KEY_MASK_POINTER)  >> KEY_OFFS_POINTER)


#define KEY_SET_ACTIVITY(key_,d_)					\
  { key_.k1&=~KEY_MASK_ACTIVITY;					\
    key_.k1|=(KEY_MASK_ACTIVITY&((unsigned long long)d_<<KEY_OFFS_ACTIVITY)); }

#define KEY_SET_REGION(key_,d_)					\
  { key_.k1&=~KEY_MASK_REGION;					\
    key_.k1|=(KEY_MASK_REGION&((unsigned long long)d_<<KEY_OFFS_REGION)); }

#define KEY_SET_TID(key_,d_)					\
  { key_.k1&=~KEY_MASK_TID;					\
    key_.k1|=(KEY_MASK_TID&((unsigned long long)d_<<KEY_OFFS_TID)); }

#define KEY_SET_CALLSITE(key_,d_)					\
  { key_.k1&=~KEY_MASK_CALLSITE;					\
    key_.k1|=(KEY_MASK_CALLSITE&((unsigned long long)d_<<KEY_OFFS_CALLSITE)); }

#define KEY_SET_DATATYPE(key_,d_)					\
  { key_.k1&=~KEY_MASK_DATATYPE;					\
    key_.k1|=(KEY_MASK_DATATYPE&((unsigned long long)d_<<KEY_OFFS_DATATYPE)); }

#define KEY_SET_OPERATION(key_,d_)					\
  { key_.k1&=~KEY_MASK_OPERATION;					\
    key_.k1|=(KEY_MASK_OPERATION&((unsigned long long)d_<<KEY_OFFS_OPERATION)); }

#define KEY_SET_SELECT(key_,d_)						\
  { key_.k1&=~KEY_MASK_SELECT;						\
    key_.k1|=(KEY_MASK_SELECT&((unsigned long long)d_<<KEY_OFFS_SELECT)); }

#define KEY_SET_BYTES(key_,d_)					\
  { key_.k2&=~KEY_MASK_BYTES;					\
    key_.k2|=(KEY_MASK_BYTES&((unsigned long long)d_<<KEY_OFFS_BYTES)); }

#define KEY_SET_RANK(key_,d_)					\
  { key_.k2&=~KEY_MASK_RANK;					\
    key_.k2|=(KEY_MASK_RANK&((unsigned long long)d_<<KEY_OFFS_RANK)); }

#define KEY_SET_POINTER(key_,d_)					\
  { key_.k2&=~KEY_MASK_POINTER;						\
    key_.k2|=(KEY_MASK_POINTER&((unsigned long long)d_<<KEY_OFFS_POINTER)); }



#define KEY_EQUAL(key1_, key2_)			\
  ((key1_.k1==key2_.k1)&&(key1_.k2==key2_.k2))

#define KEY_ASSIGN(key1_, key2_)		\
  {key1_.k1=key2_.k1; key1_.k2=key2_.k2;}

#define KEY_CLEAR(key_)				\
  {key_.k1=0; key_.k2=0;}

#define KEY_ISNULL(key_)			\
  ((key_.k1==0)&&(key_.k2==0))

#define KEY_RANDOM(key_) {			\
    unsigned long long r1, r2;			\
    r1=lrand48(); r2=lrand48();			\
    key_.k1=(r1<<32); key_.k1+=r2;		\
    r1=lrand48(); r2=lrand48();			\
    key_.k2=(r1<<32); key_.k2+=r2;		\
  }


#define KEY_HASH(key_, mod_)			\
  ((key_.k1%mod_+key_.k2%mod_)%mod_)


#define KEYPAIR_HASH(key1_, key2_, mod_)		\
  ((KEY_HASH(key1_,mod_)+KEY_HASH(key2_,mod_))%mod_)


#define KEY_FIRSTHASH(key_, size_)		\
  KEY_HASH(key_, size_)

#define KEYPAIR_FIRSTHASH(key1_, key2_, size_)	\
  KEYPAIR_HASH(key1_, key2_, size_)

#define KEY_REHASH(key_, size_, idx_, coll_)	\
  ((idx_+1)%size_)

/*
  #define KEY_REHASH(key_, size_, idx_, coll_)				\
  (KEY_HASH(key_, size_)+coll_*(1+KEY_HASH(key_,size_-2)))%size_ 
*/

#define KEYPAIR_REHASH(key1_, key2_, size_, idx_, coll_)	\
  ((idx_+1)%size_)

/*
#define KEYPAIR_REHASH(key1_, key2_, size_, idx_, coll_)	\
  (KEYPAIR_HASH(key1_, key2_, size_)+coll_*(1+KEYPAIR_HASH(key1_, key2_, size_-2)))%size_ 
*/


#define KEY_SPRINT(buf_, key_)			\
  sprintf(buf_, IPM_KEY_TYPEF, key_.k1, key_.k2)


#define KEY_SHOWBITS(key_)						\
  {									\
    unsigned long long i;						\
    for( i=0; i<64; i++ ) {						\
      printf("%d", (63-i)%10);						\
    }									\
    printf("\n");							\
    for( i=0; i<64; i++ ) {						\
      printf( ((key_.k1)&((unsigned long long)1<<(63-i)))?"1":"0" ); \
    }									\
    printf("\n");							\
    for( i=0; i<64; i++ ) {						\
      printf( ((key_.k2)&((unsigned long long)1<<(63-i)))?"1":"0" ); \
    }									\
    printf("\n");							\
  }							

#endif /* HAVE_UINT128 */


#define IPM_RANK_NULL          KEY_MAX_RANK
#define IPM_RANK_ALL           (KEY_MAX_RANK-1)
#define IPM_RANK_ANY_SOURCE    (KEY_MAX_RANK-2)


#define IPM_CALLSITE_NULL      KEY_MAX_CALLSITE
#define IPM_REGION_NULL        KEY_MAX_REGION


#define IPM_RESOURCE_BYTES_AND_RANK         0
#define IPM_RESOURCE_POINTER                1


#endif /* HASHKEY_H_INCLUDED */
