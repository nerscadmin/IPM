
#ifndef MPIIO_H_INCLUDED
#define MPIIO_H_INCLUDED

#include <mpi.h>
#include "ipm_modules.h"

int mod_mpiio_init(ipm_mod_t* mod, int flags);

typedef struct mpiiodata
{
  double iotime;
  double iotime_e;
} mpiiodata_t;

#define IPM_MPIIO_RANK_NONE_C(rank_)    rank_=IPM_MPI_RANK_NORANK;
#define IPM_MPIIO_RANK_NONE_F(rank_)    rank_=IPM_MPI_RANK_NORANK;

extern mpiiodata_t mpiiodata[MAXNUM_REGIONS];

#define IPM_MPIIO_BYTES_NONE_C( bytes_ ) \
  bytes_=0;

#define IPM_MPIIO_BYTES_COUNT_DATATYPE_C( bytes_ ) \
  {						   \
    PMPI_Type_size(datatype, &bytes_);		   \
    bytes_ *= count;				   \
  }
   
#endif /* MPIIO_H_INCLUDED */
