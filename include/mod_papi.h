
#ifndef PAPI_H_INCLUDED
#define PAPI_H_INCLUDED

#include <papi.h>

#include "ipm_modules.h"
#include "ipm_sizes.h"

#if  PAPI_VERSION>=PAPI_VERSION_NUMBER(3,9,0,0)
#else
#define   PAPI_COMPONENT_INDEX(evt_) 0 
#undef    MAXNUM_PAPI_COMPONENTS
#define   MAXNUM_PAPI_COMPONENTS 1
#endif

typedef struct
{
  int code;
  char name[MAXSIZE_PAPI_EVTNAME];
} ipm_papi_event_t;

typedef struct
{
  int evtset;
  int nevts;
  short ctr2evt[MAXNUM_PAPI_COUNTERS];
} ipm_papi_evtset_t;

extern ipm_papi_event_t papi_events[MAXNUM_PAPI_EVENTS];
extern ipm_papi_evtset_t papi_evtset[MAXNUM_PAPI_COMPONENTS];

int mod_papi_init(ipm_mod_t *mod, int flags);

int ipm_papi_read(long long *val);

double ipm_papi_gflops(long long *ctr, double time);


#endif /* PAPI_H_INCLUDED */
