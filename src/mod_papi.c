
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

/* 
   uncomment the following #define to enable multiplexing, 
   also check the number of supported counters per PAPI component 
   (MAXNUM_PAPI_COUNTERS) and the overall number of events supported 
   (MAXNUM_PAPI_EVENTS) - you might want to have both of these larger 
   when multiplexing is enabled. 
*/
/* #define USE_PAPI_MULTIPLEXING */

double flops_weight[MAXNUM_PAPI_EVENTS];
ipm_papi_event_t papi_events[MAXNUM_PAPI_EVENTS];

ipm_papi_evtset_t papi_evtset[MAXNUM_PAPI_COMPONENTS];

static PAPI_hw_info_t *hwinfo = NULL;

int ipm_papi_start();
int ipm_papi_init();

int mod_papi_init(ipm_mod_t* mod, int flags)
{
  int i, comp, rv;

  mod->state    = STATE_IN_INIT;
  mod->init     = mod_papi_init;
  mod->output   = 0;
  mod->finalize = 0; 
  mod->name     = "PAPI";

  for( comp=0; comp<MAXNUM_PAPI_COMPONENTS; comp++ ) {
    papi_evtset[comp].nevts=0;
    papi_evtset[comp].evtset=PAPI_NULL;
  }

  for(i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    papi_events[i].code=0;
//    papi_events[i].name[0]=0;
    flops_weight[i]=0.0;
  }
  /*
     sprintf(papi_events[0].name, "PAPI_FP_OPS");
     sprintf(papi_events[1].name, "PAPI_L2_TCM");
     sprintf(papi_events[1].name, "ETH0_RX_PACKETS");
     sprintf(papi_events[MAXNUM_PAPI_EVENTS-1].name, "PAPI_TOT_INS"); 
  */


  rv = ipm_papi_init();
  if(rv!=IPM_OK) { 
    mod->state = STATE_ERROR;
    return IPM_EOTHER;
  }

  /* 
    if ((hwinfo = PAPI_get_hardware_info()) != NULL) {
      printf("%d CPU's at %f Mhz.\en",hwinfo->totalcpus,hwinfo->mhz);
    
  }
  */

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
  int i, comp, rv;

  for( comp=0; comp<MAXNUM_PAPI_COMPONENTS; comp++ ) 
    {
      papi_evtset[comp].evtset=PAPI_NULL;
      rv = PAPI_create_eventset(&(papi_evtset[comp].evtset));

      if( rv!= PAPI_OK )    {
	IPMERR("PAPI: [comp %d] Error creating eventset\n", comp);
	return IPM_EOTHER;
      }

#ifdef USE_PAPI_MULTIPLEXING
      rv = PAPI_set_multiplex(papi_evtset[comp].evtset);
      if( rv!= PAPI_OK )    {
	IPMDBG("PAPI: [comp %d] Error calling set_multiplex\n", comp);
      }
#endif
    }
  
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    if( !(papi_events[i].name[0]) )
      continue;

    comp = PAPI_COMPONENT_INDEX(papi_events[i].code);

    rv = PAPI_add_event(papi_evtset[comp].evtset, 
			papi_events[i].code);
    
    if( rv!= PAPI_OK ) {
      IPMERR("PAPI: [comp %d] Error adding event to eventset: %s, skipping\n",
	     comp, papi_events[i].name);
      
    } else {
      IPMDBG("PAPI: [comp %d] Successfully added event: %s\n",
	     comp, papi_events[i].name);
      
      papi_evtset[comp].ctr2evt[papi_evtset[comp].nevts]=i;
      
      papi_evtset[comp].nevts++;
    }
  }
  
  for( comp=0; comp<MAXNUM_PAPI_COMPONENTS; comp++ ) 
    {
      if( papi_evtset[comp].nevts>0 ) {
	rv = PAPI_start(papi_evtset[comp].evtset);
	
	if( rv!=PAPI_OK ) {
	  IPMERR("PAPI: [comp %d] Error starting eventset (%d events)\n",
		 comp, papi_evtset[comp].nevts);
	  papi_evtset[comp].nevts=0;
	} else {
	  IPMDBG("PAPI: [comp %d] Successfully started eventset (%d events)\n",
		 comp, papi_evtset[comp].nevts);
	}
      }
    }

  return IPM_OK;
}

int ipm_papi_stop() 
{
  int comp, rv;
  long long ctr[MAXNUM_PAPI_EVENTS];

  for( comp=0; comp<MAXNUM_PAPI_COMPONENTS; comp++ ) 
    {
      if( papi_evtset[comp].nevts>0 ) {
	rv =  PAPI_stop(papi_evtset[comp].evtset, ctr);
	if( rv!=PAPI_OK ) {
	  IPMERR("PAPI: [comp %d] Error stopping eventset\n", comp);
	  return IPM_EOTHER;
	} else {
	  IPMDBG("PAPI: [comp %d] Successfully stopped eventset\n", comp);
	}
	
	rv =  PAPI_cleanup_eventset(papi_evtset[comp].evtset);
	if( rv!=PAPI_OK ) {
	  IPMERR("PAPI: [comp %d] Error cleaning eventset up\n", comp);
	  return IPM_EOTHER;
	} else {
	  IPMDBG("PAPI: [comp %d] Successfully cleaned eventset up\n", comp);
	}
	
	rv =  PAPI_destroy_eventset(&(papi_evtset[comp].evtset));
	if( rv!=PAPI_OK ) {
	  IPMERR("PAPI: [comp %d] Error destroying eventset\n", comp);
	  return IPM_EOTHER;
	} else {
	  IPMDBG("PAPI: [comp %d] Successfully destroyed eventset\n", comp);
	}
      }
  }
  return IPM_OK;
}

int ipm_papi_read(long long *val)
{
  int i, comp, rv;
  long long ctr[MAXNUM_PAPI_COUNTERS];

  for( comp=0; comp<MAXNUM_PAPI_COMPONENTS; comp++ ) 
    {
      if( papi_evtset[comp].nevts>0 ) {
	rv =  PAPI_read(papi_evtset[comp].evtset, ctr);
	if( rv!=PAPI_OK ) {
	  IPMERR("PAPI: [comp %d] Error reading eventset\n", comp);
	  return IPM_EOTHER;
	} else 
	  {
	    for( i=0; i<papi_evtset[comp].nevts; i++ ) {
	      val[ papi_evtset[comp].ctr2evt[i] ] = ctr[i];
	    }
	  }
      }
    }

  return IPM_OK;
}


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

