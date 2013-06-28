
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_modules.h"
#include "mod_clustering.h"
#include "calltable.h"
#include "regstack.h"
#include "hashkey.h"
#include "hashtable.h"
#include "GEN.calltable_mpi.h"
#ifdef HAVE_PAPI
#include "mod_papi.h"
#endif /* HAVE_PAPI */

#ifdef HAVE_MPI
#include "mod_mpi.h"
#endif /* HAVE_PAPI */

procstats_t mystats;

void cluster_by_structural(procstats_t *allstats);


int mod_clustering_init(ipm_mod_t* mod, int flags) 
{
  mod->state    = STATE_IN_INIT;
  mod->init     = mod_clustering_init;
  mod->output   = mod_clustering_output;
  mod->finalize = 0; 
  mod->name     = "CLUSTERING";

  mod->state    = STATE_ACTIVE;
  return IPM_OK;
}

int compare_procstat_by_npartners(const void *p1, const void *p2) {
  procstats_t *s1, *s2; 

  s1 = (procstats_t*)p1;
  s2 = (procstats_t*)p2;

  return s1->npar < s2->npar;
}

int compare_procstat_by_rank(const void *p1, const void *p2) {
  procstats_t *s1, *s2; 

  s1 = (procstats_t*)p1;
  s2 = (procstats_t*)p2;

  return ((s1->myrank)-(s2->myrank));
}

int compare_procstat_by_structural(const void *p1, const void *p2) {
  procstats_t *s1, *s2; 
  int ret;
  double d;
  
  s1 = (procstats_t*)p1;
  s2 = (procstats_t*)p2;

  /* dictionary comparison of structural 
     entries in procstat_t */

  ret = (s1->ncoll)-(s2->ncoll);
  if( ret ) return ret;

  ret = (s1->nroot)-(s2->nroot);
  if( ret ) return ret;

  ret = (s1->np2p)-(s2->np2p);
  if( ret ) return ret;

  ret = (s1->npar)-(s2->npar);
  if( ret ) return ret;

  d = (s1->parloc)-(s2->parloc);
  if( fabs(d)>1.0 ) {
    return (d>1.0?+1:-1);
  }

  d = (s1->pardist)-(s2->pardist);
  if( fabs(d)>1.0 ) {
    return (d>1.0?+1:-1);
  }

  return 0;
}



int compare_hent_by_rank(const void *p1, const void *p2) {
  ipm_hent_t *h1, *h2;
  int k1, k2;

  h1 = (ipm_hent_t*)p1;
  h2 = (ipm_hent_t*)p2;

  k1 = KEY_GET_RANK(h1->key);
  k2 = KEY_GET_RANK(h2->key);

  return (k1<k2);
}


int mod_clustering_output(ipm_mod_t* mod, int flags)
{
  int i, j;
  procstats_t *allstats;

  get_procstats(ipm_htable, &mystats);

  if( task.taskid==0 ) {
    allstats = malloc( sizeof(procstats_t)*task.ntasks );
  }

  PMPI_Gather( &mystats, sizeof(procstats_t), MPI_BYTE,
	       allstats, sizeof(procstats_t), MPI_BYTE,
	       0, MPI_COMM_WORLD );


  if( task.taskid==0 ) {
    
    cluster_by_structural(allstats);

    print_procstat(1, 0);
    for( i=0; i<task.ntasks; i++ ) 
      {
	print_procstat(0, &(allstats[i]));
      }

    qsort( allstats, task.ntasks, sizeof(procstats_t), 
	   compare_procstat_by_rank);  
  
  }


  PMPI_Scatter( allstats, sizeof(procstats_t), MPI_BYTE,
		&mystats, sizeof(procstats_t), MPI_BYTE,
		0, MPI_COMM_WORLD);
  
}


int get_procstats(ipm_hent_t *table, procstats_t *stats) 
{
  int i, j;
  int act, rank, lastrank;
  ipm_hent_t hcopy[MAXSIZE_HASH];
  
  /* make copy of hashtable and sort it by partner rank*/
  memcpy(hcopy, table, sizeof(ipm_hent_t)*MAXSIZE_HASH);
  qsort(hcopy, MAXSIZE_HASH, sizeof(ipm_hent_t), compare_hent_by_rank);

  stats->wallt=task.wtime;

#ifdef HAVE_PAPI
  stats->gflops = ipm_papi_gflops(ipm_rstack->child->ctr, stats->wallt);
#else
  stats->gflops = 0.0;
#endif 

  stats->ncoll=0;
  stats->nroot=0;
  stats->np2p=0;
  stats->npar=0;
  stats->parloc=0.0;
  stats->pardist=0.0;


  stats->tcoll=0.0;
  stats->tp2p=0.0;
  stats->dcoll=0.0;
  stats->dp2p=0.0;
  stats->clrank=-1;
  stats->myrank=task.taskid;

  
  lastrank=IPM_RANK_NULL;
  for( i=0; i<MAXSIZE_HASH; i++ ) {
    if( hcopy[i].count==0 ) 
      continue;

    rank = KEY_GET_RANK(hcopy[i].key);
    act  = KEY_GET_ACTIVITY(hcopy[i].key);
    
    if( IS_P2P_CALL(act) && 
	rank!=IPM_RANK_NULL &&
	rank!=task.taskid ) 
      {
	stats->parloc += (double)hcopy[i].count*(double)(rank-task.taskid);
	stats->pardist += hcopy[i].count*fabs(rank-task.taskid);
	
	if( rank!=lastrank ) {
	  stats->npar++;
	  lastrank=rank;
	}
      }

    /* IS_COLLECTIVE_CALL */
    if( (ipm_calltable[act].attr&DATA_COLLECTIVE) ||
	act==MPI_BARRIER_ID_GLOBAL) {

      stats->ncoll+=hcopy[i].count;

      if( rank==task.taskid ) {
	stats->nroot+=hcopy[i].count;
      }

      stats->tcoll+=hcopy[i].t_tot;
      stats->dcoll+=hcopy[i].count*KEY_GET_BYTES(hcopy[i].key);
    }
    if( IS_P2P_CALL(act) ) { 
      stats->np2p+=hcopy[i].count;
      stats->tp2p+=hcopy[i].t_tot;
      stats->dp2p+=hcopy[i].count*KEY_GET_BYTES(hcopy[i].key);
    }
  }
}


void print_procstat(int hdr, procstats_t *stats) {
  if( hdr ) {
    /*
      fprintf(stderr, "# rank cluster wallt gflops npar ncoll np2p tcoll tp2p dcoll dp2p\n");
    */
    fprintf(stderr, "# rank clrank ncoll nroot np2p npar parloc pardist ");
    fprintf(stderr, "wallt gflops tcoll tp2p dcoll dp2p");
  }
  else {
    fprintf(stderr, "%5d %5d %5d %5d %5d %5d %5.2f %5.2f ", 
	    stats->myrank, stats->clrank, 
	    stats->ncoll, stats->nroot, stats->np2p,
	    stats->npar, stats->parloc, stats->pardist);
    fprintf(stderr, "%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",
	    stats->wallt, stats->gflops, 
	    stats->tcoll, stats->tp2p, 
	    stats->dcoll, stats->dp2p);
  }
}




void cluster_by_structural(procstats_t *allstats) 
{
  int i, cl;

  qsort( allstats, task.ntasks, sizeof(procstats_t), 
	 compare_procstat_by_structural);  

  /* element 0 is a cluster center */
  allstats[0].clrank=allstats[0].myrank;
  cl=0;
  
  for( i=1; i<task.ntasks; i++ ) 
    {
      /* compare distance to previous cluster center dist(i,cl) */
      if( compare_procstat_by_structural(&(allstats[cl]), 
					 &(allstats[i])) ) {
	/* different */
	allstats[i].clrank=allstats[i].myrank;
	cl=i;
	} 
      else {
	/* similar */
	allstats[i].clrank=allstats[cl].clrank;
      }
    }
}
