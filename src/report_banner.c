
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "ipm.h"
#include "report.h"

/**
 * returns a char* with length max_length
 */
static char* strstrip(const char *str, int max_length)
{
  /*
  char *ret_string;
  
  if( str == NULL ) {
    ret_string = NULL;
  } else {
    ret_string = calloc(max_length, sizeof(char));
    strncat(ret_string, str, max_length-1);
  }
  */
  
  return (char*)str;
}

int compare_dsum(const void *p1, const void *p2) {
  gstats_t *g1, *g2;

  g1 = (gstats_t*)p1;
  g2 = (gstats_t*)p2;

  return ((g1->dsum) < (g2->dsum));
}


void ipm_print_region(FILE *f, banner_t *data, regstats_t *reg);


void ipm_print_banner(FILE *f, banner_t *data) 
{
  const struct tm *nowstruct;
  char begstr[128];
  char endstr[128];
  char tmpstr[128];
  int i, act;
  int ntasks, nthreads;

  ntasks   = data->ntasks;
  nthreads = data->nthreads;
  
  nowstruct = localtime( &(data->tstart.tv_sec) );
  strftime( begstr, 128, "%a %b %d %T %Y", nowstruct );
  nowstruct = localtime( &(data->tstop.tv_sec) );
  strftime( endstr, 128, "%a %b %d %T %Y", nowstruct );

  fprintf(f, "##IPMv%s########################################################\n",
	  IPM_VERSION);
  fprintf(f, "#\n");
  fprintf(f, "# command   : %-26s\n", 
	  data->cmdline);
  fprintf(f, "# start     : %-26s host      : %-16s\n", 
	  begstr, data->hostname);
  fprintf(f, "# stop      : %-26s wallclock : %.2f\n",
	  endstr, data->app.wallt.dmax);

  sprintf(tmpstr, "%d on %d nodes",  ntasks, data->nhosts);
  fprintf(f, "# mpi_tasks : %-26s %%comm     : %-.2f\n",
	  tmpstr, 100.0*data->app.mpi.dsum/data->app.wallt.dsum);

  if( data->flags&BANNER_HAVE_OMP ) {
  sprintf(tmpstr, "%d", nthreads);
  fprintf(f, "# omp_thrds : %-26s %%omp      : %-.2f\n",
	  tmpstr, 100.0*data->app.omp.dsum/data->app.wallt.dsum);
  }

  if( data->flags&BANNER_HAVE_POSIXIO ) {
  sprintf(tmpstr, "");
  fprintf(f, "# files     : %-26s %%i/o      : %-.2f\n",
	  tmpstr, 100.0*data->app.pio.dsum/data->app.wallt.dsum);
  }

  fprintf(f, "# mem [GB]  : %-26.2f gflop/sec : %.2f\n",
	  data->procmem.dsum, data->app.gflops.dsum);


  fprintf(f, "#\n");




  ipm_print_region(f, data, &(data->app));

  fprintf(f, "#\n");
  fprintf(f, "###################################################################\n");

  data->nregions=0;
  for( i=0; i<MAXNUM_REGIONS; i++ ) {
    if( (data->regions[i]).valid ) {
       data->nregions++;
    }
  }

/* do not print ipm_noregion if only 1 region */
  if (data->nregions>1) {

  for( i=0; i<MAXNUM_REGIONS; i++ ) {
    if( (data->regions[i]).valid ) {
      fprintf(f, "###################################################################\n");
      ipm_print_region(f, data, &(data->regions[i]));
      fprintf(f, "#\n");
      fprintf(f, "###################################################################\n");
    }
  }

  }

}


void ipm_print_region(FILE *f, banner_t *data, regstats_t *reg)
{
  int i, j, act;
  int ntasks, nthreads;

  ntasks   = data->ntasks;
  nthreads = data->nthreads;

  if( reg->name[0] ) {
    fprintf(f, "# region    :'%s'\n", reg->name);
  }
  j=0;
  for( i=0; i<MAXNUM_REGNESTING; i++ ) {
    if( reg->nesting[i][0] ) j++;
    else break;
  }
  if( j>0 ) {
    fprintf(f, "# nesting   :");
    for( i=j; i>=0; i-- ) {
      if( reg->nesting[i][0] ) {
	fprintf(f, "'%s' %s ", reg->nesting[i], i>0?"->":"");
      }
    }
    fprintf(f, "\n");
  }
 

  fprintf(f, "#           :       [total]        <avg>          min          max\n");
  fprintf(f, "# wallclock :    %10.2f   %10.2f   %10.2f   %10.2f \n", 
	  reg->wallt.dsum, (reg->wallt.dsum)/((double)ntasks), 
	  (reg->wallt.dmin), reg->wallt.dmax);
  
  if( data->flags&BANNER_HAVE_MPI ) {
    fprintf(f, "# MPI       :    %10.2f   %10.2f   %10.2f   %10.2f \n", 
	    reg->mpi.dsum, reg->mpi.dsum/(double)ntasks, 
	    reg->mpi.dmin, reg->mpi.dmax);
  }
  if( data->flags&BANNER_HAVE_OMP ) {
    fprintf(f, "# OMP       :    %10.2f   %10.2f   %10.2f   %10.2f \n", 
	    reg->omp.dsum, reg->omp.dsum/(double)ntasks, 
	    reg->omp.dmin, reg->omp.dmax);
    fprintf(f, "# OMP idle  :    %10.2f   %10.2f   %10.2f   %10.2f \n", 
	    reg->ompi.dsum/(double)nthreads,
            reg->ompi.dsum/(double)ntasks/(double)nthreads, 
	    reg->ompi.dmin/(double)nthreads, reg->ompi.dmax/(double)nthreads);
  }
  if( data->flags&BANNER_HAVE_POSIXIO ) {
    fprintf(f, "# I/O       :    %10.2f   %10.2f   %10.2f   %10.2f \n",
	    reg->pio.dsum, reg->pio.dsum/(double)ntasks, 
	    reg->pio.dmin, reg->pio.dmax);
  }
  if( data->flags&BANNER_HAVE_CUDA ) {
    fprintf(f, "# CUDA      :    %10.2f   %10.2f   %10.2f   %10.2f \n",
	    reg->cuda.dsum, reg->cuda.dsum/(double)ntasks, 
	    reg->cuda.dmin, reg->cuda.dmax);
  }
  if( data->flags&BANNER_HAVE_CUBLAS ) {
    fprintf(f, "# CUBLAS    :    %10.2f   %10.2f   %10.2f   %10.2f \n",
	    reg->cublas.dsum, reg->cublas.dsum/(double)ntasks, 
	    reg->cublas.dmin, reg->cublas.dmax);
  }
  if( data->flags&BANNER_HAVE_CUFFT ) {
    fprintf(f, "# CUFFT     :    %10.2f   %10.2f   %10.2f   %10.2f \n",
	    reg->cufft.dsum, reg->cufft.dsum/(double)ntasks, 
	    reg->cufft.dmin, reg->cufft.dmax);
  }
  
  fprintf(f, "# %%wall     :\n");
  if( data->flags&BANNER_HAVE_MPI ) {
    fprintf(f, "#   MPI     :                 %10.2f   %10.2f   %10.2f \n", 
	    reg->mpip.dsum/(double)data->ntasks, 
	    reg->mpip.dmin, reg->mpip.dmax); 
  }
  if( data->flags&BANNER_HAVE_OMP ) {
    fprintf(f, "#   OMP     :                 %10.2f   %10.2f   %10.2f \n", 
	    reg->ompp.dsum/(double)data->ntasks, 
	    reg->ompp.dmin, reg->ompp.dmax); 
  }
  if( data->flags&BANNER_HAVE_POSIXIO ) {
    fprintf(f, "#   I/O     :                 %10.2f   %10.2f   %10.2f \n", 
	    reg->piop.dsum/(double)ntasks, 
	    reg->piop.dmin, reg->piop.dmax);
  }
  if( data->flags&BANNER_HAVE_CUDA ) {
    fprintf(f, "#   CUDA    :                 %10.2f   %10.2f   %10.2f \n", 
	    reg->cudap.dsum/(double)ntasks, 
	    reg->cudap.dmin, reg->cudap.dmax);
  }
  if( data->flags&BANNER_HAVE_CUBLAS ) {
    fprintf(f, "#   CUBLAS  :                 %10.2f   %10.2f   %10.2f \n", 
	    reg->cublasp.dsum/(double)ntasks, 
	    reg->cublasp.dmin, reg->cublasp.dmax);
  }
  if( data->flags&BANNER_HAVE_CUFFT ) {
    fprintf(f, "#   CUFFT   :                 %10.2f   %10.2f   %10.2f \n", 
	    reg->cufftp.dsum/(double)ntasks, 
	    reg->cufftp.dmin, reg->cufftp.dmax);
  }
   
  fprintf(f, "# #calls    :\n");
  if( data->flags&BANNER_HAVE_MPI ) {
    fprintf(f, "#   MPI     :    "
	    "%10"IPM_COUNT_TYPEF"   %10"IPM_COUNT_TYPEF 
	    "   %10"IPM_COUNT_TYPEF"   %10" IPM_COUNT_TYPEF "\n", 
	    reg->mpi.nsum, (reg->mpi.nsum)/ntasks, 
	    reg->mpi.nmin, reg->mpi.nmax);
  }
  if( data->flags&BANNER_HAVE_POSIXIO ) {
    fprintf(f, "#   I/O     :    "	    
	    "%10"IPM_COUNT_TYPEF"   %10"IPM_COUNT_TYPEF 
	    "   %10"IPM_COUNT_TYPEF"   %10" IPM_COUNT_TYPEF "\n",
	    reg->pio.nsum, reg->pio.nsum/data->ntasks, 
	    reg->pio.nmin, reg->pio.nmax);
    fprintf(f, "# I/O [GB]  :    %10.2f   %10.2f   %10.2f   %10.2f \n",
	    reg->pio_GiB.dsum, reg->pio_GiB.dsum/(double)ntasks,
	    reg->pio_GiB.dmin, reg->pio_GiB.dmax);
  }
  
  fprintf(f, "# mem [GB]  :    %10.2f   %10.2f   %10.2f   %10.2f \n", 
	  data->procmem.dsum, data->procmem.dsum/(double)ntasks,
	  data->procmem.dmin, data->procmem.dmax);

  if( data->flags&BANNER_HAVE_ENERGY ) {
    double joules =  reg->energy.dsum;
    double cpu_joules =  reg->cpu_energy.dsum;
    double mem_joules =  reg->mem_energy.dsum;
    double other_joules =  reg->other_energy.dsum;
    double kwh = joules / (double)(3600000.0);
    double cpu_kwh = cpu_joules / (double)(3600000.0);
    double mem_kwh = mem_joules / (double)(3600000.0);
    double other_kwh = other_joules / (double)(3600000.0);
    fprintf(f,"#\n# Per Node Energy Data:\n");
    fprintf(f, "# energy (j):    %10.2lf   %10.2lf  %10.2lf   %10.2lf \n", joules, joules/ntasks, reg->energy.dmin, reg->energy.dmax);
    fprintf(f, "#    -cpu   :    %10.2lf   %10.2lf  %10.2lf   %10.2lf \n", cpu_joules, cpu_joules/ntasks, reg->cpu_energy.dmin, reg->cpu_energy.dmax);
    fprintf(f, "#    -mem   :    %10.2lf   %10.2lf  %10.2lf   %10.2lf \n", mem_joules, mem_joules/ntasks, reg->mem_energy.dmin, reg->mem_energy.dmax);
    fprintf(f, "#    -other :    %10.2lf   %10.2lf  %10.2lf   %10.2lf \n", other_joules, other_joules/ntasks, reg->other_energy.dmin, reg->other_energy.dmax);
    fprintf(f, "#\n");
    fprintf(f, "# power (w) :    %10.2lf   %10.2lf  %10.2lf   %10.2lf \n",
            (joules/reg->wallt.dsum) * (task.nhosts), joules/reg->wallt.dsum,
            reg->energy.dmin/(reg->wallt.dsum/(double)task.ntasks), reg->energy.dmax/(reg->wallt.dsum/(double)task.ntasks));
    fprintf(f, "#    -cpu   :    %10.2lf   %10.2lf  %10.2lf   %10.2lf \n",
            (cpu_joules/reg->wallt.dsum) * (task.nhosts), cpu_joules/reg->wallt.dsum,
            reg->cpu_energy.dmin/(reg->wallt.dsum/(double)task.ntasks), reg->cpu_energy.dmax/(reg->wallt.dsum/(double)task.ntasks));
    fprintf(f, "#    -mem   :    %10.2lf   %10.2lf  %10.2lf   %10.2lf \n",
            (mem_joules/reg->wallt.dsum) * (task.nhosts), mem_joules/reg->wallt.dsum,
            reg->mem_energy.dmin/(reg->wallt.dsum/(double)task.ntasks), reg->mem_energy.dmax/(reg->wallt.dsum/(double)task.ntasks));
    fprintf(f, "#    -other :    %10.2lf   %10.2lf  %10.2lf   %10.2lf \n",
            (other_joules/reg->wallt.dsum) * (task.nhosts), other_joules/reg->wallt.dsum,
            reg->other_energy.dmin/(reg->wallt.dsum/(double)task.ntasks), reg->other_energy.dmax/(reg->wallt.dsum/(double)task.ntasks));

    fprintf(f, "# kWh       :    %10lf   %10lf  %10lf   %10lf \n", kwh, kwh/ntasks, reg->energy.dmin/3600000.0, reg->energy.dmax/3600000.0);
    fprintf(f, "#    -cpu   :    %10lf   %10lf  %10lf   %10lf \n", cpu_kwh, cpu_kwh/ntasks, reg->cpu_energy.dmin/3600000.0, reg->cpu_energy.dmax/3600000.0);
    fprintf(f, "#    -mem   :    %10lf   %10lf  %10lf   %10lf \n", mem_kwh, mem_kwh/ntasks, reg->mem_energy.dmin/3600000.0, reg->mem_energy.dmax/3600000.0);
    fprintf(f, "#    -other :    %10lf   %10lf  %10lf   %10lf \n", other_kwh, other_kwh/ntasks, reg->other_energy.dmin/3600000.0, reg->other_energy.dmax/3600000.0);
  }

  if( data->flags&BANNER_FULL ) 
    {
      /* sort by total time in calls */
      qsort( reg->fullstats, MAXSIZE_CALLTABLE, sizeof(gstats_t), 
	     compare_dsum);
      
      fprintf(f, "#\n");
      fprintf(f, "#                             [time]        [count]        <%%wall>\n");
      for( i=0; i<MAXSIZE_CALLTABLE; i++ ) {
	if( reg->fullstats[i].nsum>0 ) {
	  act = reg->fullstats[i].activity;
	  
	  fprintf(f, "# %-20s    %10.2f     %10"IPM_COUNT_TYPEF"     %10.2f\n",
		  strstrip(data->calltable[act],20),
		  reg->fullstats[i].dsum, reg->fullstats[i].nsum, 
		  100.0*(reg->fullstats[i].dsum)/(reg->wallt.dsum) );
	}
      }
    }
}
