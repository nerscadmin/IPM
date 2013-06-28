
#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#define IPM_ADDR_TYPE    unsigned long long int
#define IPM_ADDR_TYPEF  "%p"

/*
#define IPM_COUNT_TYPE      unsigned long long int
#define IPM_COUNT_TYPEF     "llu"
#define IPM_COUNT_MPITYPE   MPI_UNSIGNED_LONG_LONG
*/

#define IPM_COUNT_TYPE        unsigned long int
#define IPM_COUNT_TYPEF       "lu"
#define IPM_COUNT_MPITYPE     MPI_UNSIGNED_LONG
#define IPM_COUNT_MAX         4294967295UL

#define IPM_RANK_TYPE        int 
#define IPM_RANK_TYPEF       "d" 




#endif /* TYPES_H_INCLUDED */
