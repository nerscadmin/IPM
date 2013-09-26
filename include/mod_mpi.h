
#ifndef MOD_MPI_H_INCLUDED
#define MOD_MPI_H_INCLUDED

#include <mpi.h>
#include "ipm_modules.h"

int mod_mpi_init(ipm_mod_t* mod, int flags);
int mod_mpi_output(ipm_mod_t* mod, int flags);
int mod_mpi_finalize(ipm_mod_t* mod, int flags);

/* enables collection of datatype and operations for collectives */
//#define IPM_COLLECTIVE_DETAILS 1

#define MPI_STATUS_SOURCE  MPI_SOURCE 

#define IPM_MPI_RANK_ALLRANKS 0
#define IPM_MPI_RANK_NORANK   0

typedef struct mpidata
{
  double mtime;
  double mtime_e;
} mpidata_t;

extern mpidata_t mpidata[MAXNUM_REGIONS];
extern MPI_Group ipm_world_group;

#define IPM_MPI_RANK_NONE_C(rank_)    rank_=IPM_MPI_RANK_NORANK;
#define IPM_MPI_RANK_NONE_F(rank_)    rank_=IPM_MPI_RANK_NORANK;
#define IPM_MPI_RANK_ALL_C(rank_)     rank_=IPM_MPI_RANK_ALLRANKS;
#define IPM_MPI_RANK_ALL_F(rank_)     rank_=IPM_MPI_RANK_ALLRANKS;
#define IPM_MPI_RANK_ROOT_C(rank_)    rank_=root;
#define IPM_MPI_RANK_ROOT_F(rank_)    rank_=*root;
#define IPM_MPI_RANK_SRC_C(rank_)     rank_=src;
#define IPM_MPI_RANK_SRC_F(rank_)     rank_=*src;
#define IPM_MPI_RANK_DEST_C(rank_)    rank_=dest;
#define IPM_MPI_RANK_DEST_F(rank_)    rank_=*dest;

#define IPM_MPI_RANK_STATUS_C(rank_)             \
  if(status && (status)!=MPI_STATUS_IGNORE )     \
    {rank_ = status->MPI_STATUS_SOURCE;}

#define IPM_MPI_RANK_STATUS_F(rank_)             \
  if(status && (status)!=MPI_STATUS_IGNORE )     \
    {rank_ = status->MPI_STATUS_SOURCE;}


/* depends on the MPI implementation, one of
   count, _count, size, val1 */
#ifndef MPI_STATUS_COUNT
#define MPI_STATUS_COUNT  count
#endif

#define IPM_MPI_BYTES_NONE_C(size_)		\
  size_=0;

#define IPM_MPI_BYTES_SCOUNT_C(size_)					\
  {									\
    PMPI_Type_size(stype, &size_);					\
    size_*=scount;							\
  }

//
//  This is added to support the all case of gather since all of the sbuf must be ignored, but data is sent
//
//  For the gather and gatherv
#define IPM_MPI_BYTES_SCOUNT_GA_C(size_)				\
  {									\
    int tcount;								\
    if (sbuf != MPI_IN_PLACE) {PMPI_Type_size(stype, &size_); tcount = scount;}	\
    else {size_ = 0; tcount = 0;}					\
    size_*=tcount;							\
  }

//  For the reductions
#define IPM_MPI_BYTES_SCOUNT_RE_C(size_)				\
  {									\
    PMPI_Type_size(stype, &size_);					\
    size_*=scount;							\
  }
//  For the allgather
#define IPM_MPI_BYTES_SCOUNT_ALL_C(size_)				\
  {									\
    int tcount;								\
    if (sbuf != MPI_IN_PLACE) {PMPI_Type_size(stype, &size_); tcount = scount;}	\
    else {PMPI_Type_size(rtype, &size_); tcount = rcount;}		\
    size_*=tcount;							\
  }
// For the allgetherv
#define IPM_MPI_BYTES_SCOUNT_ALLV_C(size_)				\
  {									\
    int tcount;								\
    int myrank;								\
    PMPI_Comm_rank(comm_in, &myrank);					\
    if (sbuf != MPI_IN_PLACE) {PMPI_Type_size(stype, &size_); tcount = scount;}	\
    else {PMPI_Type_size(rtype, &size_); tcount = rcounts[myrank];}     \
    size_*=tcount;							\
  }

//
//  This needs updated once we figure out how to determine MPI_IN_PLACE for fortran
//
#define IPM_MPI_BYTES_SCOUNT_F(size_)					\
  {									\
    PMPI_Type_size(stype, &size_); tcount = (*scount);			\
    size_*=tcount;							\
  }

#define IPM_MPI_BYTES_RCOUNT_C(size_)					\
  {									\
    PMPI_Type_size(rtype, &size_);					\
    size_*=rcount;							\
  }

//
//  THis is for the scatter functions
//
#define IPM_MPI_BYTES_RCOUNT_SC_C(size_)				\
  {									\
    int tcount;								\
    if (rbuf != MPI_IN_PLACE) {PMPI_Type_size(rtype, &size_); tcount = rcount;}	\
    else { size_ = 0; tcount = 0;}					\
    size_*=tcount;							\
  }

#define IPM_MPI_BYTES_RCOUNT_F(size_)  \
  PMPI_Type_size(rtype, &size_);       \
  size_*=(*rcount);

#define IPM_MPI_BYTES_RCOUNTI_C(size_)		\
  size_=0;

#define IPM_MPI_BYTES_STATUS_C(size_)             \
  if(status && (status)!=MPI_STATUS_IGNORE )      \
    {size_ = status->MPI_STATUS_COUNT;}

#define IPM_MPI_BYTES_STATUS_F(size_)            \
  if(status && (status)!=MPI_STATUS_IGNORE )     \
    {size_ = status->MPI_STATUS_COUNT;}

#define IPM_MPI_BYTES_STATUSES_C(size_)				\
  {								\
    int i;							\
    size_ = 0;							\
    if( statuses && statuses!=MPI_STATUSES_IGNORE ) {		\
      for( i=0; i<num; i++ ) {					\
	size_+=statuses[i].MPI_STATUS_COUNT; 			\
      }								\
    }								\
  }


#define IPM_MPI_BYTES_SCOUNTI_C(size_)		\
  {						\
    int myrank;					\
    PMPI_Comm_rank(comm_in, &myrank);		\
    PMPI_Type_size(stype, &size_);		\
    size_=scounts[myrank]*size_;		\
  }

#define IPM_MPI_BYTES_SCOUNTI_F(size_)	   \
  {					   \
    int myrank;				   \
    PMPI_Comm_rank(*comm_in, &myrank);     \
    PMPI_Type_size(*stype, &size_);	   \
    size_=scounts[myrank]*size_;	   \
  }

#define IPM_MPI_BYTES_SCOUNTS_C(size_)		\
  {						\
    int myrank;					\
    PMPI_Comm_rank(comm_in, &myrank);		\
    PMPI_Type_size(stype, &size_);   \
    size_=scounts[myrank]*size_;     \
  }

#define IPM_MPI_TRACE(t1, t2, fid_, fname_, rank_, size_, reg_, csite_) \
  if( task.tracefile && task.tracestate) {                              \
    char buf[80];                                                       \
    switch(rank_) {                                                     \
    case IPM_RANK_NULL:                                                 \
      sprintf(buf, ""); /* "rank=NULL");*/                              \
      break;                                                            \
    case IPM_RANK_ALL:                                                  \
      sprintf(buf, ""); /* "rank=ALL");*/                               \
      break;                                                            \
    case IPM_RANK_ANY_SOURCE:                                           \
      sprintf(buf, "rank=ANY_SOURCE");                                  \
      break;                                                            \
    default:                                                            \
      sprintf(buf, "rank=%d", rank_);                                   \
      break;                                                            \
    }                                                                   \
    fprintf(task.tracefile, "% .9f % .9f %s %dB %s reg=%d cs=%d\n",     \
            t1, t2, fname_, size_, buf, reg_, csite_);                  \
    fflush(task.tracefile);                                             \
  }


#define IS_P2P_CALL(call_)				\
  ((ipm_calltable[call_].attr&DATA_RX) ||		\
   (ipm_calltable[call_].attr&DATA_TX) ||		\
   (ipm_calltable[call_].attr&DATA_TXRX))


#define IS_COLLECTIVE_CALL_ID(id_)				\
  (id_==MPI_BCAST_ID)          || (id_==MPI_REDUCE_ID)	   ||	\
  (id_==MPI_REDUCE_SCATTER_ID) || (id_==MPI_SCAN_ID)	   ||	\
  (id_==MPI_SCATTER_ID)        || (id_==MPI_SCATTERV_ID)   ||	\
  (id_==MPI_GATHER_ID)         || (id_==MPI_GATHERV_ID)	   ||	\
  (id_==MPI_ALLGATHER_ID)      || (id_==MPI_ALLGATHERV_ID) ||	\
  (id_==MPI_ALLTOALL_ID)       || (id_==MPI_ALLTOALLV_ID)  ||	\
  (id_==MPI_ALLREDUCE_ID)


#define IS_COLLECTIVE_CALL(id_)				\
  (id_==MPI_Gather)      


#ifdef NEED_C2F_MACROS 

#define MPI_Status_c2f(c_,f_) *((MPI_Status *)f_)=*((MPI_Status *)c_)
#define MPI_Status_f2c(f_,c_) *((MPI_Status *)c_)=*((MPI_Status *)f_)

#endif /* NEED_C2F_MACROS */


/*
 * monitoring MPI datatypes and reduction operations
 */
#define IPM_MPI_MAX       1
#define IPM_MPI_MIN       2
#define IPM_MPI_SUM       3
#define IPM_MPI_PROD      4
#define IPM_MPI_LAND      5
#define IPM_MPI_BAND      6
#define IPM_MPI_LOR       7
#define IPM_MPI_BOR       8
#define IPM_MPI_LXOR      9
#define IPM_MPI_BXOR     10
#define IPM_MPI_MINLOC   11
#define IPM_MPI_MAXLOC   12

extern char* ipm_mpi_op[MAXNUM_MPI_OPS];


#define IPM_MPI_CHAR                     1
#define IPM_MPI_BYTE                     2
#define IPM_MPI_SHORT                    3
#define IPM_MPI_INT                      4
#define IPM_MPI_LONG                     5
#define IPM_MPI_FLOAT                    6
#define IPM_MPI_DOUBLE                   7
#define IPM_MPI_UNSIGNED_CHAR            8
#define IPM_MPI_UNSIGNED_SHORT           9
#define IPM_MPI_UNSIGNED                10
#define IPM_MPI_UNSIGNED_LONG           11
#define IPM_MPI_LONG_DOUBLE             12
#define IPM_MPI_LONG_LONG_INT           13
#define IPM_MPI_FLOAT_INT               14
#define IPM_MPI_LONG_INT                15
#define IPM_MPI_DOUBLE_INT              16
#define IPM_MPI_SHORT_INT               17
#define IPM_MPI_2INT                    18
#define IPM_MPI_LONG_DOUBLE_INT         19
#define IPM_MPI_PACKED                  20
#define IPM_MPI_UB                      21
#define IPM_MPI_LB                      22
#define IPM_MPI_REAL                    23
#define IPM_MPI_INTEGER                 24
#define IPM_MPI_LOGICAL                 25
#define IPM_MPI_DOUBLE_PRECISION        26
#define IPM_MPI_COMPLEX                 27
#define IPM_MPI_DOUBLE_COMPLEX          28
#define IPM_MPI_INTEGER1                29
#define IPM_MPI_INTEGER2                30
#define IPM_MPI_INTEGER4                31
#define IPM_MPI_REAL4                   32
#define IPM_MPI_REAL8                   33
#define IPM_MPI_2INTEGER                34
#define IPM_MPI_2REAL                   35
#define IPM_MPI_2DOUBLE_PRECISION       36
#define IPM_MPI_2COMPLEX                37
#define IPM_MPI_2DOUBLE_COMPLEX         38

extern char* ipm_mpi_type[MAXNUM_MPI_TYPES];


/* 
   we can't use a switch statement here, because MPI_Op might not 
   be an integer but a struct or something else 
*/
#define MPIOP_TO_IPMOP( mpiop, ipmop ) {			\
    if( mpiop==MPI_MAX )         ipmop=IPM_MPI_MAX;		\
    else if( mpiop==MPI_MIN )    ipmop=IPM_MPI_MIN;		\
    else if( mpiop==MPI_SUM )    ipmop=IPM_MPI_SUM;		\
    else if( mpiop==MPI_PROD )   ipmop=IPM_MPI_PROD;		\
    else if( mpiop==MPI_LAND )   ipmop=IPM_MPI_LAND;		\
    else if( mpiop==MPI_BAND )   ipmop=IPM_MPI_BAND;		\
    else if( mpiop==MPI_LOR )    ipmop=IPM_MPI_LOR;		\
    else if( mpiop==MPI_BOR )    ipmop=IPM_MPI_BOR;		\
    else if( mpiop==MPI_LXOR )   ipmop=IPM_MPI_LXOR;		\
    else if( mpiop==MPI_BXOR )   ipmop=IPM_MPI_BXOR;		\
    else if( mpiop==MPI_MINLOC ) ipmop=IPM_MPI_MINLOC;		\
    else if( mpiop==MPI_MAXLOC ) ipmop=IPM_MPI_MAXLOC;		\
    else ipmop=0;						\
  }


#define MPITYPE_TO_IPMTYPE( mpitype, ipmtype ) {			\
    if( mpitype==MPI_CHAR )                   ipmtype=IPM_MPI_CHAR;	\
    else if( mpitype==MPI_BYTE )              ipmtype=IPM_MPI_BYTE;	\
    else if( mpitype==MPI_SHORT )             ipmtype=IPM_MPI_SHORT;	\
    else if( mpitype==MPI_INT )               ipmtype=IPM_MPI_INT;		\
    else if( mpitype==MPI_LONG )              ipmtype=IPM_MPI_LONG;	\
    else if( mpitype==MPI_FLOAT )             ipmtype=IPM_MPI_FLOAT;	\
    else if( mpitype==MPI_DOUBLE )            ipmtype=IPM_MPI_DOUBLE;	\
    else if( mpitype==MPI_UNSIGNED_CHAR )     ipmtype=IPM_MPI_UNSIGNED_CHAR; \
    else if( mpitype==MPI_UNSIGNED_SHORT )    ipmtype=IPM_MPI_UNSIGNED_SHORT; \
    else if( mpitype==MPI_UNSIGNED )          ipmtype=IPM_MPI_UNSIGNED;	\
    else if( mpitype==MPI_UNSIGNED_LONG )     ipmtype=IPM_MPI_UNSIGNED_LONG; \
    else if( mpitype==MPI_LONG_DOUBLE )       ipmtype=IPM_MPI_LONG_DOUBLE;	\
    else if( mpitype==MPI_LONG_LONG_INT )     ipmtype=IPM_MPI_LONG_LONG_INT; \
    else if( mpitype==MPI_FLOAT_INT )         ipmtype=IPM_MPI_FLOAT_INT;	\
    else if( mpitype==MPI_LONG_INT )          ipmtype=IPM_MPI_LONG_INT;	\
    else if( mpitype==MPI_DOUBLE_INT )        ipmtype=IPM_MPI_DOUBLE_INT;	\
    else if( mpitype==MPI_SHORT_INT )         ipmtype=IPM_MPI_SHORT_INT;	\
    else if( mpitype==MPI_2INT )              ipmtype=IPM_MPI_2INT;	\
    else if( mpitype==MPI_LONG_DOUBLE_INT )   ipmtype=IPM_MPI_LONG_DOUBLE_INT; \
    else if( mpitype==MPI_PACKED )            ipmtype=IPM_MPI_PACKED;		\
    else if( mpitype==MPI_UB )                ipmtype=IPM_MPI_UB;			\
    else if( mpitype==MPI_LB )                ipmtype=IPM_MPI_LB;			\
    else if( mpitype==MPI_REAL )              ipmtype=IPM_MPI_REAL;		\
    else if( mpitype==MPI_INTEGER )           ipmtype=IPM_MPI_INTEGER;		\
    else if( mpitype==MPI_LOGICAL )           ipmtype=IPM_MPI_LOGICAL;		\
    else if( mpitype==MPI_DOUBLE_PRECISION )  ipmtype=IPM_MPI_DOUBLE_PRECISION; \
    else if( mpitype==MPI_COMPLEX )           ipmtype=IPM_MPI_COMPLEX;		\
    else if( mpitype==MPI_DOUBLE_COMPLEX )    ipmtype=IPM_MPI_DOUBLE_COMPLEX; \
    else if( mpitype==MPI_INTEGER1 )          ipmtype=IPM_MPI_INTEGER1;	\
    else if( mpitype==MPI_INTEGER2 )          ipmtype=IPM_MPI_INTEGER2;	\
    else if( mpitype==MPI_INTEGER4 )          ipmtype=IPM_MPI_INTEGER4;	\
    else if( mpitype==MPI_REAL4 )             ipmtype=IPM_MPI_REAL4;		\
    else if( mpitype==MPI_REAL8 )             ipmtype=IPM_MPI_REAL8;		\
    else if( mpitype==MPI_2INTEGER )          ipmtype=IPM_MPI_2INTEGER;	\
    else if( mpitype==MPI_2REAL )             ipmtype=IPM_MPI_2REAL;		\
    else if( mpitype==MPI_2DOUBLE_PRECISION ) ipmtype=IPM_MPI_2DOUBLE_PRECISION; \
    else if( mpitype==MPI_2COMPLEX )          ipmtype=IPM_MPI_2COMPLEX;	\
    else if( mpitype==MPI_2DOUBLE_COMPLEX )   ipmtype=IPM_MPI_2DOUBLE_COMPLEX;	\
    else ipmtype=0;							\
  }


static const int mask3bits[32] = 
  {0x1,     0x3,     0x7,     0x7<<1,  0x7<<2,  0x7<<3,
   0x7<<4,  0x7<<5,  0x7<<6,  0x7<<7,  0x7<<8,  0x7<<9,
   0x7<<10, 0x7<<11, 0x7<<12, 0x7<<13, 0x7<<14, 0x7<<15,
   0x7<<16, 0x7<<17, 0x7<<18, 0x7<<19, 0x7<<20, 0x7<<21,
   0x7<<22, 0x7<<23, 0x7<<23, 0x7<<25, 0x7<<26, 0x7<<27,
   0x7<<28, 0x7<<29};


#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n

static const char logtable256[256] = 
  {
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
  };


#define KEEP_ONLY_HIGH_3BITS(val)				\
  {						\
    int lg, tmp;				\
    						\
    if (tmp = val >> 24) {				\
      lg = 24 + logtable256[tmp];			\
    } else if (tmp = val >> 16) {			\
      lg = 16 + logtable256[tmp];			\
    } else if (tmp = val >> 8) {			\
      lg = 8 + logtable256[tmp];			\
    } else {						\
      lg = logtable256[val];				\
    }							\
    val=val&mask3bits[lg];				\
  }


#endif 
