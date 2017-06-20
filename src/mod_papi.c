
#ifdef HAVE_PAPI

#include <papi.h>
#include <stdio.h>
#include <string.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_sizes.h"
#include "ipm_debug.h"
#include "ipm_modules.h"
#include "mod_papi.h"
#include "report.h"

// tallen: previous versions of this code naively assumed all events were 
// compatible with all components. This is not the case. The current code
// only supports core events located by default at index 0. Additional code
// will have to be added to support other events. Events can look up what
// components they are supported on - this would be the better way to determine
// which events to monitor.

/* 
   uncomment the following #define to enable multiplexing, 
   also check the number of supported counters per PAPI component 
   (MAXNUM_PAPI_COUNTERS) and the overall number of events supported 
   (MAXNUM_PAPI_EVENTS) - you might want to have both of these larger 
   when multiplexing is enabled. 
*/
//#define USE_PAPI_MULTIPLEXING

double flops_weight[MAXNUM_PAPI_EVENTS];
ipm_papi_event_t papi_events[MAXNUM_PAPI_EVENTS];

ipm_papi_evtset_t papi_evtset[MAXNUM_PAPI_COMPONENTS];


int ipm_papi_start();
int ipm_papi_init();

int mod_papi_xml(ipm_mod_t* mod, void* ptr, struct region* reg)
{
    int res = 0;
    const PAPI_hw_info_t *hwinfo = PAPI_get_hardware_info();
    // time here for compatibility with standard - papi has no interactive regions
    res += ipm_printf(ptr, "<module name=\"PAPI\" time=\"0.0\" ncpu=\"%d\" nnodes=\"%d\" totalcpus=\"%d\"\
 vendor=\"%d\" vendor_string=\"%s\" model=\"%d\" model_string=\"%s\" revision=\"%f\" mhz=\"%f\"></module>\n",
                       hwinfo->ncpu, hwinfo->nnodes, hwinfo->totalcpus, hwinfo->vendor, hwinfo->vendor_string,
                       hwinfo->model, hwinfo->model_string, hwinfo->revision, hwinfo->mhz);

    return res;
}

int mod_papi_init(ipm_mod_t* mod, int flags)
{
  int i, comp, rv;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_papi_init;
  mod->xml      = mod_papi_xml;
  mod->output   = 0;
  mod->finalize = 0;
  mod->name     = "PAPI";

  // intialize components
  for( comp=0; comp<MAXNUM_PAPI_COMPONENTS; comp++ ) {
    papi_evtset[comp].nevts=0;
    papi_evtset[comp].evtset=PAPI_NULL;
  }

  // clear events
  for(i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    papi_events[i].code=0;
    flops_weight[i]=0.0;
  }

  rv = ipm_papi_init();
  if(rv!=IPM_OK) {
    mod->state = STATE_ERROR;
    return IPM_EOTHER;
  }

  rv = ipm_papi_start();
  if(rv!=IPM_OK) {
    mod->state = STATE_ERROR;
    return IPM_EOTHER;
  }

  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}


int ipm_papi_init()
{
    int i, rv;

    rv=PAPI_library_init( PAPI_VER_CURRENT );

    if( rv!=PAPI_VER_CURRENT && rv>0 ) {
        IPMDBG("PAPI library version mismatch\n");
        return IPM_EOTHER;
    } else if( rv<0 ) {
        IPMERR("PAPI initialization error (%d)\n", rv);
        return IPM_EOTHER;
    }      

    if( PAPI_VERSION>=PAPI_VERSION_NUMBER(3,9,0,0) ) {
        IPMDBG("Detected component PAPI (PAPI-C). Max %d components supported by IPM\n", 
        MAXNUM_PAPI_COMPONENTS);
    } else {
        IPMDBG("Detected classic PAPI. One default (CPU) component supported\n");
    }

#ifdef USE_PAPI_MULTIPLEXING
    rv=PAPI_multiplex_init();
    if( rv!=PAPI_OK ) {
        IPMERR("PAPI_multiplex_init() failed.\n");
    }
#endif

    /* translate PAPI event names to codes and check validity */
    for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
        if( papi_events[i].name[0] ) {
            PAPI_event_name_to_code(papi_events[i].name, &(papi_events[i].code));

            if( PAPI_query_event(papi_events[i].code)!=PAPI_OK )
            {
                IPMERR("PAPI: Event name-to-code error: %s, ignoring\n",
                papi_events[i].name);
                papi_events[i].name[0]=0;
                papi_events[i].code=0;
            }
            else
            {
                if( !strcmp(papi_events[i].name, "PAPI_FP_OPS") ) {
                    flops_weight[i]=1.0;
                }
                IPMDBG("PAPI: Successfully registered event: %s\n", 
                papi_events[i].name);
            }
        }
    }

    return IPM_OK;
}


int ipm_papi_start()
{
    int i, rv;
    const PAPI_component_info_t* cmpinfo = NULL;

    // count events during all contexts (user, kernel, etc)
    rv = PAPI_set_domain(PAPI_DOM_ALL);
    cmpinfo = PAPI_get_component_info( 0 );
    if ( cmpinfo == NULL )
        IPMDBG("BUG: No core component found by PAPI\n");
    papi_evtset[0].evtset=PAPI_NULL;
    rv = PAPI_create_eventset(&(papi_evtset[0].evtset));
    // attach eventset to appropriate component - currently just cpu
    // tyler: seems to be necessary for multiplexing
    rv = PAPI_assign_eventset_component( papi_evtset[0].evtset, 0 );

    if( rv!= PAPI_OK )    {
        IPMERR("PAPI: [comp %d] Error creating eventset: %s\n", 0, PAPI_strerror(rv));
        return IPM_EOTHER;
    }

#ifdef USE_PAPI_MULTIPLEXING
    rv = PAPI_set_multiplex(papi_evtset[0].evtset);
    if( rv!= PAPI_OK ) {
        IPMDBG("PAPI: [comp %d] Error calling set_multiplex\n", 0);
    }
#endif

    // add events
    for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
        if( !(papi_events[i].name[0]) )
            continue;

        rv = PAPI_add_event(papi_evtset[0].evtset, papi_events[i].code);
        // if an event is listed more than once, the -8 lacking-hardware error
        // will be given. behavior undefined for papi and ipm
        if( rv!= PAPI_OK ) {
            IPMERR("PAPI: [comp %d] Error adding event to eventset: %s, %s, skipping\n",
            0, papi_events[i].name, PAPI_strerror(rv));

        } else {
            IPMDBG("PAPI: [comp %d] Successfully added event: %s\n",
            0, papi_events[i].name);

            papi_evtset[0].ctr2evt[papi_evtset[0].nevts]=i;

            papi_evtset[0].nevts++;
        }
    }

    // start papi
    if( papi_evtset[0].nevts>0 ) {
        rv = PAPI_start(papi_evtset[0].evtset);
        if( rv!=PAPI_OK ) {
            IPMERR("PAPI: [comp %d] Error starting eventset (%d events)\n",
            0, papi_evtset[0].nevts);
            papi_evtset[0].nevts=0;
        } else {
            IPMDBG("PAPI: [comp %d] Successfully started eventset (%d events)\n",
            0, papi_evtset[0].nevts);
        }
    }

    return IPM_OK;
}

int ipm_papi_stop() 
{
    int rv;
    long long ctr[MAXNUM_PAPI_EVENTS];

    if( papi_evtset[0].nevts>0 ) {
        rv =  PAPI_stop(papi_evtset[0].evtset, ctr);
        if( rv!=PAPI_OK ) {
            IPMERR("PAPI: [comp %d] Error stopping eventset\n", 0);
            return IPM_EOTHER;
        } else {
            IPMDBG("PAPI: [comp %d] Successfully stopped eventset\n", 0);
        }

        rv =  PAPI_cleanup_eventset(papi_evtset[0].evtset);
        if( rv!=PAPI_OK ) {
            IPMERR("PAPI: [comp %d] Error cleaning eventset up\n", 0);
            return IPM_EOTHER;
        } else {
            IPMDBG("PAPI: [comp %d] Successfully cleaned eventset up\n", 0);
        }

        rv =  PAPI_destroy_eventset(&(papi_evtset[0].evtset));
        if( rv!=PAPI_OK ) {
            IPMERR("PAPI: [comp %d] Error destroying eventset\n", 0);
            return IPM_EOTHER;
        } else {
            IPMDBG("PAPI: [comp %d] Successfully destroyed eventset\n", 0);
        }
    }
    return IPM_OK;
}

int ipm_papi_read(long long *val)
{
    int i, rv;
    long long ctr[MAXNUM_PAPI_COUNTERS];

    if( papi_evtset[0].nevts>0 ) {
        rv =  PAPI_read(papi_evtset[0].evtset, ctr);
        if( rv!=PAPI_OK ) {
            IPMERR("PAPI: [comp %d] Error reading eventset\n", 0);
            return IPM_EOTHER;
        } else {
            for( i=0; i<papi_evtset[0].nevts; i++ ) {
                val[ papi_evtset[0].ctr2evt[i] ] = ctr[i];
            }
        }
    }

    return IPM_OK;
}

// tyler:
// this function's current use is strange; used to report during banner call,
// fails to provide the right value as it is always called from rank==0 once
// per process
double ipm_papi_gflops(long long *ctr, double time)
{
  int i;
  double gflops=0.0;
  double sum=0.0;

  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    gflops += ((double)ctr[i]) * flops_weight[i];
    sum += flops_weight[i];
  }
  
  gflops /= 1.0e9;

  if( sum>0.0 ) {
    return gflops / (time*sum);
  } else {
    return 0.0;
  }
  
}



#endif /* HAVE_PAPI */

