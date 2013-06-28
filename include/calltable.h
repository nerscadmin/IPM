
#ifndef  CALLTABLE_H_INCLUDED
#define  CALLTABLE_H_INCLUDED

#include "ipm_sizes.h"

typedef struct 
{
  char               *name;
  unsigned long long attr;
} ipm_call_t;

extern ipm_call_t ipm_calltable[MAXSIZE_CALLTABLE];


#define RANK_ALL             (0x1ULL<<0)
#define RANK_DEST            (0x1ULL<<1)
#define RANK_NONE            (0x1ULL<<2)
#define RANK_ROOT            (0x1ULL<<3)
#define RANK_SRC             (0x1ULL<<4)
#define RANK_STATUS          (0x1ULL<<5)

#define DATA_NONE            (0x1ULL<<6)
#define DATA_COLLECTIVE      (0x1ULL<<7)
#define DATA_RX              (0x1ULL<<8)
#define DATA_TX              (0x1ULL<<9)
#define DATA_TXRX            (0x1ULL<<10)

#define BYTES_NONE           (0x1ULL<<11)
#define BYTES_NMEMB          (0x1ULL<<12)
#define BYTES_COUNT          (0x1ULL<<13)
#define BYTES_CHAR           (0x1ULL<<14)
#define BYTES_RETURN_NMEMB   (0x1ULL<<15)
#define BYTES_RETURN_COUNT   (0x1ULL<<16)
#define BYTES_RETURN_EOF     (0x1ULL<<17)

#define BYTES_SCOUNT         (0x1ULL<<18)
#define BYTES_STATUS         (0x1ULL<<19)
#define BYTES_RCOUNT         (0x1ULL<<20)
#define BYTES_STATUSI        (0x1ULL<<21)
#define BYTES_STATUSES       (0x1ULL<<22)
#define BYTES_RCOUNTI        (0x1ULL<<23)
#define BYTES_SCOUNTI        (0x1ULL<<24)
#define BYTES_SCOUNTS        (0x1ULL<<25)

#define BYTES_COUNT_DATATYPE (0x1ULL<<26)
#define BYTES_EXTENT         (0x1ULL<<27)
#define BYTES_SIZE           (0x1ULL<<28)
#define BYTES_WIDTH_HEIGHT   (0x1ULL<<29)

#define BYTES_NX             (0x1ULL<<30)
#define BYTES_NXNY           (0x1ULL<<31)
#define BYTES_NXNYNZ         (0x1ULL<<32)

#define BYTES_MNK            (0x1ULL<<33)
#define BYTES_NELEMSIZE      (0x1ULL<<34)
//
//  added to support the allgather which needs to use rtype vs stype when there is MPI_IN_PLACE
// 
#define BYTES_SCOUNT_GA      (0x1ULL<<35)
#define BYTES_SCOUNT_RE      (0x1ULL<<36)
#define BYTES_SCOUNT_ALL     (0x1ULL<<37)
#define BYTES_SCOUNT_ALLV    (0x1ULL<<38)
#define BYTES_RCOUNT_SC      (0x1ULL<<38)


#endif /* CALLTABLE_H_INCLUDED */

