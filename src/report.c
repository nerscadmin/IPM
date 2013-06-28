
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_types.h"
#include "ipm_time.h"
#include "hashtable.h"
#include "hashkey.h"
#include "report.h"
#include "regstack.h"
#include "calltable.h"

#ifdef HAVE_MPI
#include <mpi.h>
#include "GEN.calltable_mpi.h"
#endif
#ifdef HAVE_POSIXIO
#include "GEN.calltable_posixio.h"
#endif
#ifdef HAVE_MEM
#include "GEN.calltable_mem.h"
#endif
#ifdef HAVE_PAPI
#include "mod_papi.h"
#endif
#ifdef HAVE_OMPTRACEPOINTS
#include "mod_omptracepoints.h"
#endif
#ifdef HAVE_CUDA
#include "mod_cuda.h"
#include "GEN.calltable_cuda.h"
#endif
#ifdef HAVE_CUBLAS
#include "mod_cublas.h"
#include "GEN.calltable_cublas.h"
#endif
#ifdef HAVE_CUFFT
#include "mod_cufft.h"
#include "GEN.calltable_cufft.h"
#endif



/* 
 * compute global statistics for hashtable statistics
 * - min of min 
 * - max of max
 * - sum of sum
 */
void gstats_hent(ipm_hent_t hent, gstats_t *global) 
{
#ifdef HAVE_MPI
  IPM_REDUCE( &hent.t_tot, &(global->dmin), 1, 
	      MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  
  IPM_REDUCE( &hent.t_tot, &(global->dmax), 1, 
	      MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  
  IPM_REDUCE( &hent.t_tot, &(global->dsum), 1, 
	      MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  IPM_REDUCE( &hent.count, &(global->nmin), 1, 
	      IPM_COUNT_MPITYPE, MPI_MIN, 0, MPI_COMM_WORLD);

  IPM_REDUCE( &hent.count, &(global->nmax), 1, 
	      IPM_COUNT_MPITYPE, MPI_MAX, 0, MPI_COMM_WORLD);

  IPM_REDUCE( &hent.count, &(global->nsum), 1, 
	      IPM_COUNT_MPITYPE, MPI_SUM, 0, MPI_COMM_WORLD);
/* on the cray with IPM_REPORT =full this causes a message
   flood and a crash - to prevent this put in a barrier
   Nick 10 June 2010 */
  PMPI_Barrier(MPI_COMM_WORLD);
#else
  global->dmin = hent.t_tot;
  global->dmax = hent.t_tot;
  global->dsum = hent.t_tot;
  global->nmin = hent.count;
  global->nmax = hent.count;
  global->nsum = hent.count;
#endif
}

/* global statistics for a single double value */
void gstats_double(double val, gstats_t *global) 
{
#ifdef HAVE_MPI
  IPM_REDUCE( &val, &(global->dmin), 1,
	      MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  IPM_REDUCE( &val, &(global->dmax), 1,
	      MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  IPM_REDUCE( &val, &(global->dsum), 1,
	      MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
#else /* HAVE_MPI */
  global->dmin=val;
  global->dmax=val;
  global->dsum=val;
#endif /* HAVE_MPI */

  global->nmin=1;
  global->nmax=1;
  global->nsum=1;
}

/* global statistics for a single count */
void gstats_count(IPM_COUNT_TYPE count, gstats_t *global) 
{
#ifdef HAVE_MPI
  IPM_REDUCE( &count, &(global->nmin), 1,
	      IPM_COUNT_MPITYPE, MPI_MIN, 0, MPI_COMM_WORLD);
  IPM_REDUCE( &count, &(global->nmax), 1,
	      IPM_COUNT_MPITYPE, MPI_MAX, 0, MPI_COMM_WORLD);
  IPM_REDUCE( &count, &(global->nsum), 1,
	      IPM_COUNT_MPITYPE, MPI_SUM, 0, MPI_COMM_WORLD);
#else
  global->nmin=count;
  global->nmax=count;
  global->nsum=count;
#endif
  global->dmin=0.0;
  global->dmax=0.0;
  global->dsum=0.0;
}


void clear_region_stats(regstats_t *stats) 
{
  int i;

  GSTATS_CLEAR( stats->gflops );
  GSTATS_CLEAR( stats->wallt );
  GSTATS_CLEAR( stats->mpi );
  GSTATS_CLEAR( stats->mpip );
  GSTATS_CLEAR( stats->pio );
  GSTATS_CLEAR( stats->piop );
  GSTATS_CLEAR( stats->omp );
  GSTATS_CLEAR( stats->ompp );
  GSTATS_CLEAR( stats->ompi );
  GSTATS_CLEAR( stats->cuda );
  GSTATS_CLEAR( stats->cudap );
  GSTATS_CLEAR( stats->cublas );
  GSTATS_CLEAR( stats->cublasp );
  GSTATS_CLEAR( stats->cufft );
  GSTATS_CLEAR( stats->cufftp );

  for(i=0; i<MAXSIZE_CALLTABLE; i++ ) {
    GSTATS_CLEAR(stats->fullstats[i]);
  }
}



void compute_local_region_stats(region_t *reg, regstats_t *stats, int incl, int first) 
{
  int i, noreg;
  region_t * tmp;
  scanspec_t spec;
  double wallt, gflops;
  scanstats_t hmpi, hpio, homp, hompi;
  scanstats_t hcuda, hcublas, hcufft;
  ipm_hent_t hall[MAXSIZE_CALLTABLE];
  
  HENT_CLEAR(hmpi.hent);
  HENT_CLEAR(hpio.hent);
  HENT_CLEAR(homp.hent);
  HENT_CLEAR(hompi.hent);

  HENT_CLEAR(hcuda.hent);
  HENT_CLEAR(hcublas.hent);
  HENT_CLEAR(hcufft.hent);

  /* is this for ipm_noregion? */
  noreg=0;
  if( reg==ipm_rstack->child && !incl )
    noreg=1;
  
  for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
    stats->fullstats[i].activity=i;
    HENT_CLEAR(hall[i]);
  }

  scanspec_unrestrict_all(&spec);
  if( noreg || reg!=ipm_rstack->child ) {
    scanspec_restrict_region(&spec, reg->id, reg->id);
  }

  /* get local statistics */
  wallt = reg->wtime;

#ifdef HAVE_PAPI
  gflops = ipm_papi_gflops(reg->ctr, wallt);
#else
  gflops = 0.0;
#endif

#ifdef HAVE_MPI  
  scanspec_restrict_activity( &spec, 
			      MPI_MINID_GLOBAL, MPI_MAXID_GLOBAL);
  htable_scan( ipm_htable, &hmpi, spec );
#endif 

#ifdef HAVE_POSIXIO
  scanspec_restrict_activity( &spec, 
			      POSIXIO_MINID_GLOBAL, POSIXIO_MAXID_GLOBAL);
  htable_scan( ipm_htable, &hpio, spec );
#endif

#ifdef HAVE_OMPTRACEPOINTS
  scanspec_restrict_activity( &spec, 
			      OMP_PARALLEL_ID_GLOBAL, OMP_PARALLEL_ID_GLOBAL );
  htable_scan( ipm_htable, &homp, spec );
  
  scanspec_restrict_activity( &spec, 
			      OMP_IDLE_ID_GLOBAL, OMP_IDLE_ID_GLOBAL );
  htable_scan( ipm_htable, &hompi, spec );
#endif 

#ifdef HAVE_CUDA
  scanspec_restrict_activity( &spec, 
			      CUDA_MINID_GLOBAL, CUDA_MAXID_GLOBAL);
  htable_scan( ipm_htable, &hcuda, spec );
#endif

#ifdef HAVE_CUBLAS
  scanspec_restrict_activity( &spec, 
			      CUBLAS_MINID_GLOBAL, CUBLAS_MAXID_GLOBAL);
  htable_scan( ipm_htable, &hcublas, spec );
#endif

#ifdef HAVE_CUFFT
  scanspec_restrict_activity( &spec, 
			      CUFFT_MINID_GLOBAL, CUFFT_MAXID_GLOBAL);
  htable_scan( ipm_htable, &hcufft, spec );
#endif

  /* -- compute per-call local stats for full banner -- */ 
  if( task.flags&FLAG_REPORT_FULL )
    {
      scanspec_unrestrict_activity( &spec );
      htable_scan_full(ipm_htable, hall, spec);
    }  

  if( first ) 
    {
      GSTATS_SET( stats->wallt, wallt, 1 );
      GSTATS_SET( stats->gflops, gflops, 1 );
      GSTATS_SET( stats->mpi,   hmpi.hent.t_tot,  hmpi.hent.count );
      GSTATS_SET( stats->pio,   hpio.hent.t_tot,  hpio.hent.count );
      GSTATS_SET( stats->omp,   homp.hent.t_tot,  homp.hent.count );
      GSTATS_SET( stats->ompi,  hompi.hent.t_tot, hompi.hent.count );

      GSTATS_SET( stats->cuda,   hcuda.hent.t_tot,   hcuda.hent.count );
      GSTATS_SET( stats->cublas, hcublas.hent.t_tot, hcublas.hent.count );
      GSTATS_SET( stats->cufft,  hcufft.hent.t_tot,  hcufft.hent.count );

      if( task.flags&FLAG_REPORT_FULL ) {
	for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
	  GSTATS_SET( stats->fullstats[i], hall[i].t_tot, hall[i].count );
	}  
      }
    }
  else 
    {
      /* wallt is already inclusive, so nothing to do here */
      /*  Appro
      *
      *   Since wallt is inclusive, gflops is inclusive
      */
      GSTATS_ADD( stats->mpi,   hmpi.hent.t_tot,  hmpi.hent.count );
      GSTATS_ADD( stats->pio,   hpio.hent.t_tot,  hpio.hent.count );
      GSTATS_ADD( stats->omp,   homp.hent.t_tot,  homp.hent.count );
      GSTATS_ADD( stats->ompi,  hompi.hent.t_tot, hompi.hent.count );

      GSTATS_ADD( stats->cuda,   hcuda.hent.t_tot,   hcuda.hent.count );
      GSTATS_ADD( stats->cublas, hcublas.hent.t_tot, hcublas.hent.count );
      GSTATS_ADD( stats->cufft,  hcufft.hent.t_tot,  hcufft.hent.count );

      if( task.flags&FLAG_REPORT_FULL ) {
	for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
	  GSTATS_ADD( stats->fullstats[i], hall[i].t_tot, hall[i].count );
	}  
      }
    }
    
  if( incl && reg!=ipm_rstack->child ) {
    tmp=reg->child;
    while(tmp) {
      compute_local_region_stats(tmp, stats, incl, 0);
      tmp=tmp->next;
    }
  }  
}

void compute_region_stats(region_t *reg, regstats_t *stats, int incl) 
{
  int i, noreg;
  region_t *tmp;
  double mpip, piop, ompp, gflops, wallt;
  double cudap, cublasp, cufftp;
  ipm_hent_t hmpi, hpio, homp, hompi;
  ipm_hent_t hcuda, hcublas, hcufft;
  ipm_hent_t hall[MAXSIZE_CALLTABLE];

  /* is this for ipm_noregion? */
  noreg=0;
  if( reg==ipm_rstack->child && !incl )
    noreg=1;

  /* -- compute local stats -- */
  compute_local_region_stats(reg, stats, incl, 1);

  wallt  = stats->wallt.dsum;
  gflops = stats->gflops.dsum;

  /* handle special case of walltime for 
     ipm_noregion */
  if( noreg ) {
    tmp=reg->child;
    while(tmp) {
      wallt-=tmp->wtime;
      tmp=tmp->next;
    }
  }

  /* compute percentages */
  mpip = 100.0 * (stats->mpi.dsum) / wallt;
  GSTATS_SET( stats->mpip, mpip, 1 );

  piop = 100.0 * (stats->pio.dsum) / wallt;
  GSTATS_SET( stats->piop, piop, 1 );

  ompp = 100.0 * (stats->omp.dsum) / wallt;
  GSTATS_SET( stats->ompp, ompp, 1 );

  cudap = 100.0 * (stats->cuda.dsum) / wallt;
  GSTATS_SET( stats->cudap, cudap, 1 );

  cublasp = 100.0 * (stats->cublas.dsum) / wallt;
  GSTATS_SET( stats->cublasp, cublasp, 1 );

  cufftp = 100.0 * (stats->cufft.dsum) / wallt;
  GSTATS_SET( stats->cufftp, cufftp, 1 );


  hmpi.t_tot = stats->mpi.dsum;
  hmpi.count = stats->mpi.nsum;
  
  hpio.t_tot = stats->pio.dsum;
  hpio.count = stats->pio.nsum;

  homp.t_tot = stats->omp.dsum;
  homp.count = stats->omp.nsum;

  hompi.t_tot = stats->ompi.dsum;
  hompi.count = stats->ompi.nsum;

  hcuda.t_tot = stats->cuda.dsum;
  hcuda.count = stats->cuda.nsum;

  hcublas.t_tot = stats->cublas.dsum;
  hcublas.count = stats->cublas.nsum;

  hcufft.t_tot = stats->cufft.dsum;
  hcufft.count = stats->cufft.nsum;

  if( task.flags&FLAG_REPORT_FULL ) {
    for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
      hall[i].t_tot = stats->fullstats[i].dsum;
      hall[i].count = stats->fullstats[i].nsum;
    }  
  }

  /* -- compute global stats -- */
  gstats_double( wallt,  &(stats->wallt) );
  gstats_double( gflops, &(stats->gflops) );
  gstats_double( mpip,   &(stats->mpip) );
  gstats_double( piop,   &(stats->piop) );
  gstats_double( ompp,   &(stats->ompp) );

  gstats_hent( hmpi,  &(stats->mpi) );
  gstats_hent( hpio,  &(stats->pio) );
  gstats_hent( homp,  &(stats->omp) );
  gstats_hent( hompi, &(stats->ompi) );

  gstats_hent( hcuda,   &(stats->cuda) );
  gstats_hent( hcublas, &(stats->cublas) );
  gstats_hent( hcufft,  &(stats->cufft) );

  if( task.flags&FLAG_REPORT_FULL ) {
    for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
      gstats_hent(hall[i], &(stats->fullstats[i]));
    }  
  }
}

banner_t banner;

void ipm_banner(FILE *f)
{
  ipm_hent_t stats_mpi;
  ipm_hent_t stats_pio;
  ipm_hent_t stats_omp;
  ipm_hent_t stats_ompi;
  double wallt, gflops, mpip, piop, ompp;
  ipm_hent_t stats_all[MAXSIZE_CALLTABLE];  
  int i, j;

  for( i=0; i<MAXNUM_REGIONS; i++ )  {
      banner.regions[i].valid=0;
      banner.regions[i].name[0]='\0';
      for( j=0; j<MAXNUM_REGNESTING; j++ ) {
	banner.regions[i].nesting[j][0]='\0';
      }
  }

  
  banner.flags=0;
#ifdef HAVE_MPI
  banner.flags|=BANNER_HAVE_MPI;
#endif
#ifdef HAVE_POSIXIO
  banner.flags|=BANNER_HAVE_POSIXIO;
#endif
#ifdef HAVE_OMPTRACEPOINTS
  banner.flags|=BANNER_HAVE_OMP;
#endif
#ifdef HAVE_CUDA
  banner.flags|=BANNER_HAVE_CUDA;
#endif
#ifdef HAVE_CUBLAS
  banner.flags|=BANNER_HAVE_CUBLAS;
#endif
#ifdef HAVE_CUFFT
  banner.flags|=BANNER_HAVE_CUFFT;
#endif
  
  if( task.flags&FLAG_REPORT_FULL ) 
    {
      banner.flags|=BANNER_FULL;
      for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
	banner.calltable[i]=ipm_calltable[i].name;
      }
    }

  gstats_double( task.procmem, &(banner.procmem) );
  
  /* --- compute statistics for whole app --- */
  clear_region_stats( &(banner.app) );
  compute_region_stats(ipm_rstack->child, &(banner.app), 1);

  for( j=2; j<MAXNUM_REGIONS; j++ ) {
    region_t *reg=0;
    region_t *tmp=0;
    
    reg = rstack_find_region_by_id(ipm_rstack, j);
    if( reg ) {
      if( !((task.flags)&FLAG_NESTED_REGIONS) && 
	  reg->parent!=ipm_rstack->child ) {
	continue;
      }
      
      banner.regions[j].valid=1;
      strncpy(banner.regions[j].name, reg->name, MAXSIZE_REGLABEL);
	
      /* record the nesting */
      tmp=reg;
      for( i=0; i<MAXNUM_REGNESTING; i++ ) {
	if(!tmp || tmp==task.rstack) 
	  break;
	strncpy(banner.regions[j].nesting[i], tmp->name, MAXSIZE_REGLABEL);
	tmp=tmp->parent;
      }
      
      clear_region_stats( &(banner.regions[j]) );
      compute_region_stats(reg, &(banner.regions[j]), 1);	
    }
  } 

  /* --- compute statistics for ipm_noregion --- */
  clear_region_stats( &(banner.regions[1]) );
  compute_region_stats(ipm_rstack->child, &(banner.regions[1]), 0);
  sprintf(banner.regions[1].name, "ipm_noregion");
  banner.regions[1].valid=1;  
  
  
#ifdef HAVE_MPI
  PMPI_Barrier(MPI_COMM_WORLD);
#endif

  if( task.taskid==0 ) {

    banner.app.valid=1;
    banner.app.name[0]='\0';    

    /* --- prepare banner with data from task 0 --- */

    banner.tstart   = task.t_start;
    banner.tstop    = task.t_stop;
    banner.ntasks   = task.ntasks;
    banner.nhosts   = task.nhosts;
    banner.nthreads = 1;

#ifdef HAVE_OMPTRACEPOINTS
    banner.nthreads = maxthreads;
#endif /* HAVE_OMPTRACEPOINTS */

    strcpy(banner.cmdline, (const char*)task.exec_cmdline);
    strcpy(banner.hostname, task.hostname);

    /* --- print it --- */
    ipm_print_banner(f, &banner);
  }
}

