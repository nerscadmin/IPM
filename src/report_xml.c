
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>       
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "ipm.h"
#include "ipm_types.h"
#include "hashtable.h"
#include "calltable.h"
#include "ipm_time.h"
#include "report.h"
#include "regstack.h"

#ifdef HAVE_MPI
#include <mpi.h>
#include "mod_mpi.h"
#include "GEN.calltable_mpi.h"
#else
#define IS_P2P_CALL(call_) 0
#endif

#ifdef HAVE_PAPI
#include "mod_papi.h"
#endif 

#ifdef HAVE_CLUSTERING
#include "mod_clustering.h"
#endif

#ifdef HAVE_OMPTRACEPOINTS
#include "mod_omptracepoints.h"
#endif

#ifdef HAVE_POSIXIO
#include "mod_posixio.h"
#include "GEN.calltable_posixio.h"
#endif

#ifdef HAVE_MPIIO
#include "mod_mpiio.h"
#include "GEN.calltable_mpiio.h"
#endif

#ifdef HAVE_CUDA
#include "mod_cuda.h"
#endif

/* #define PRINT_ENV 1 */

char logfname[MAXSIZE_FILENAME] = {'\0'};

/* map internal region IDs to IDs used in the xml file
   this is neccessary to handle the case where we only 
   print the first level of regions */
int internal2xml[MAXNUM_REGIONS];

int xml_region(void *ptr, taskdata_t *t, region_t *reg, ipm_hent_t *htab);
int xml_regions(void *ptr, taskdata_t *t, struct region *reg, ipm_hent_t *htab);

/* write an IPM1 compatible XML profile: */
/* 
<?xml version="1.0" encoding="iso-8859-1"?>
  <ipm_job_profile>
    <task ... mpi_rank="" ... mpi_size="" ... username="" >
      <job> </job>
      <host mach_name>""</host>
      <perf> </perf>
      <switch> </switch>
      <cmdline> </cmdline>
      <env> </env>
      ...
      <env> </env>
      <ru_s_ti> </ru_s_ti>
      <regions >
        <region>
           <hpm>
              <counter > </counter>
           </hpm>
        </region>
      </regions>
      <hash nlog="" kney="">
        <hent> </hent>
        ...
      </hash>
      <internal/>
    </task>
 </ipm_job_profile>
*/


/*
 * The output routines use this ipm_printf routine which either
 * outputs to a FILE pointer or writes to a memory buffer.
 * The selection between the two modes happens by setting the 
 * global variable print_selector. 
5~ */

#define PRINT_TO_FILE      0
#define PRINT_TO_BUFFER    1

static int print_selector=PRINT_TO_FILE;
static int print_offset=0;
static unsigned long print_flags=0;

int ipm_printf(void *ptr, const char* format, ...)
{
  int rv;
  char *buf;
  va_list ap;

  va_start (ap,format);

  switch(print_selector) {
  case PRINT_TO_FILE:
    rv = vfprintf((FILE*)ptr,format,ap);
    break;
    
  case PRINT_TO_BUFFER:
    buf = (char*)(((char*)ptr)+print_offset);
    rv = vsprintf(buf,format,ap);
    print_offset+=rv;
    break;
  }
  va_end(ap);

  return rv;
}

int xml_calltable(void *ptr) {
  int i, j, offs, range, res=0;
  int nmod, ncalls;
 
  /* count number of modules */
  nmod=0; 
  for( i=0; i<MAXNUM_MODULES; i++ ) 
    {
      range = modules[i].ct_range;
      if( !(modules[i].name) || range==0 ) 
	continue;
      
      nmod++;
    }

  res+=ipm_printf(ptr, "<calltable nsections=\"%d\" >\n", 
		  nmod );

  for( i=0; i<MAXNUM_MODULES; i++ ) 
    {
      offs  = modules[i].ct_offs;
      range = modules[i].ct_range;
      
      if( !(modules[i].name) || range==0 ) 
	continue;
      
      /* count number of calls in module */
      ncalls=0; 
      for( j=0; j<range; j++ ) 
	{
	  if( !(ipm_calltable[offs+j].name) ) 
	    continue;
	  ncalls++;
	}

      res+=ipm_printf(ptr, "<section module=\"%s\" nentries=\"%d\" >\n",
		      modules[i].name, ncalls );

      for( j=0; j<range; j++ ) 
	{
	  if( !(ipm_calltable[offs+j].name) ) 
	    continue;
	  res += ipm_printf(ptr, "<entry name=\"%s\" />\n", 
			    ipm_calltable[offs+j].name);
	}
      res+=ipm_printf(ptr, "</section>\n");
    }

  res+=ipm_printf(ptr, "</calltable>\n");

  return res;
}


int xml_profile_header(void *ptr) {
  int res=0;
  res += ipm_printf(ptr, "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
  res += ipm_printf(ptr, "<ipm_job_profile>\n");
  
  res += xml_calltable(ptr);
  return res;
}

int xml_profile_footer(void *ptr) {
  int res=0;
  res += ipm_printf(ptr, "</ipm_job_profile>\n");
  return res;
}

/* FIXME: ipm_version, cookie, flags */
int xml_task_header(void *ptr, taskdata_t *t) {
  int res=0;

  res+=ipm_printf(ptr, "<task ipm_version=\"%s\" cookie=\"%s\" mpi_rank=\"%d\" mpi_size=\"%d\" "
		  "stamp_init=\"%.6f\" stamp_final=\"%.6f\" username=\"%s\" allocationname=\"%s\" "
		  "flags=\"%lld\" pid=\"%d\" >\n",
		  IPM_VERSION, "nocookie", t->taskid, t->ntasks,
		  IPM_TIMEVAL(t->t_start), IPM_TIMEVAL(t->t_stop), 
		  t->user, t->allocation, (long long)0, t->pid);
  return res;
}

int xml_task_footer(void *ptr, taskdata_t *t) {
  int res=0;
  res+=ipm_printf(ptr, "</task>\n");
  return res;
}

/* FIXME: cookie, code */
int xml_job(void *ptr, taskdata_t *t) {
  int res=0;
  res += ipm_printf(ptr, "<job nhosts=\"%d\" ntasks=\"%d\" start=\"%ld\" final=\"%ld\" "
		    "cookie=\"%s\" code=\"%s\" >%s</job>\n",
		    t->nhosts, t->ntasks, t->t_start.tv_sec, 
		    t->t_stop.tv_sec,
		    "nocookie", "unknown",
		    t->jobid);
  return res;
}

int xml_host(void *ptr, taskdata_t *t) {
  int res=0;
  res += ipm_printf(ptr, "<host mach_name=\"%s\" mach_info=\"%s\" >%s</host>\n",
		    t->mach_name, t->mach_info, 
		    t->hostname);
  return res;
}

int xml_perf(void *ptr, taskdata_t *t) {
  int i, res;
  double procmem;
  double gflops;
  region_t *reg;

  /*
  assert(t); 
  assert(t->rstack);
  assert(t->rstack->child);
  reg = t->rstack->child;  */ /* this is ipm_main */

  reg = &ipm_app;
  
  procmem = task.procmem;
#ifdef HAVE_PAPI
  gflops = ipm_papi_gflops(reg->ctr, reg->wtime);
#else
  gflops = 0.0;
#endif 
  
  res=0;

  /* OLD IPM2 version:
  res += ipm_printf(ptr, "<perf wtime=\"%.5e\" utime=\"%.5e\" "
		    "stime=\"%.5e\" mtime=\"%.5e\" "
		    "iotime=\"%.5e\" omptime=\"%.5e\" ompidletime=\"%.5e\" "
		    "gflop=\"%.5e\" gbyte=\"%.5e\" ></perf>\n",
		    IPM_TIMEVAL(t->t_stop)-IPM_TIMEVAL(t->t_start), 
		    t->utime, t->stime, t->mtime, 
		    t->iotime, t->omptime, t->ompidletime,
		    gflops, procmem);
  */

  /* retained the mtime entry here for backwards compatibility */
  res += ipm_printf(ptr, "<perf wtime=\"%.5e\" utime=\"%.5e\" "
		    "stime=\"%.5e\" mtime=\"%.5e\" "
		    "gflop=\"%.5e\" gbyte=\"%.5e\" >",
		    IPM_TIMEVAL(t->t_stop)-IPM_TIMEVAL(t->t_start), 
		    t->utime, t->stime, t->mtime,
		    gflops, procmem);
  res += ipm_printf(ptr, "</perf>\n");

  return res;
}

int xml_modules(void *ptr, taskdata_t *t, region_t *reg) {
  int i, res=0;
  int nmod;

  nmod=0;
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    if( !(modules[i].name) || !(modules[i].xml) ) 
      continue;

    nmod++;
  }
 
  res += ipm_printf(ptr, "<modules nmod=\"%d\">\n", nmod);

  /* print the contribution to the task-wide <modules> entry
     or the region-specific <modules> entry for each module */
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    if( !(modules[i].name) || !(modules[i].xml) ) 
      continue;

    res+=modules[i].xml(&(modules[i]), ptr, reg); 
  }
  
  res += ipm_printf(ptr, "</modules>\n");

  return res;
}

/* FIXME: bytes_tx, bytes_rx */
int xml_switch(void *ptr, taskdata_t *t) {
  int res=0;
  res+=ipm_printf(ptr, "<switch bytes_tx=\"0.00000e+00\" bytes_rx=\"0.00000e+00\" ></switch>\n");
  return res;
}

int xml_cmdline(void *ptr, taskdata_t *t) {
  int res=0;
  res+=ipm_printf(ptr, "<cmdline realpath=\"%s\" md5sum=\"%s\" >%s</cmdline>\n",
		  t->exec_realpath, t->exec_md5sum, t->exec_cmdline);
  return res;
}

int xml_env(void *ptr, taskdata_t *t) {
  int res, len;
  char line[512]; 
  FILE *f;

  res=0;
 
#ifdef PRINT_ENV
  res+=ipm_printf(ptr, "<env>\n");
  f = fopen("/proc/self/maps", "r");
  while( f && fgets(line, sizeof(line), f)!=0) 
    {
      len = strlen(line);
      if( len>0 && line[len-1]=='\n' ) 
	line[len-1]=0;
      
      res+=ipm_printf(ptr, "<ldmap>%s<ldmap>\n", line);
    }
  fclose(f);
  res+=ipm_printf(ptr, "</env>\n");
#endif

  /*
    ipm_printf(ptr, "<env>LESSKEY=/etc/lesskey.bin</env>\n");
  */
  return res;
}


int xml_ru(void *ptr, taskdata_t *t) {
  int res=0;
  /*
    ipm_printf(f, "<ru_s_ti>1.2000e-02 6.0003e-02 0 0 0 0 18322 0 0 0 0 0 0 0 9 0</ru_s_ti>\n");
    ipm_printf(f, "<ru_s_tf>9.8486e+00 4.3203e-01 0 0 0 0 124631 4 0 0 0 0 0 0 218 3534</ru_s_tf>\n");
   ipm_printf(f, "<ru_c_ti>0.0000e+00 0.0000e+00 0 0 0 0 0 0 0 0 0 0 0 0 0 0</ru_c_ti>\n");
   ipm_printf(f, "<ru_c_tf>0.0000e+00 0.0000e+00 0 0 0 0 0 0 0 0 0 0 0 0 0 0</ru_c_tf>\n");
  */
  return res;
}

int xml_call_mask(void *ptr, taskdata_t *t) {
  int res=0;
  /*
    ipm_printf(ptr, "<call_mask >\n");
    ipm_printf(ptr, "</call_mask >\n");
  */
  return res;
}


int xml_internal(void *ptr, taskdata_t *t) {
  int res=0;
  
  res+=ipm_printf(ptr, "<internal rank=\"%d\" log_i=\"%12.6f\" log_t=\"%.4e\" "
		  "report_delta=\"%.4e\" fname=\"%s\" logrank=\"%d\" ></internal>\n",
		  t->taskid,
		  IPM_TIMEVAL(t->t_stop),
		  IPM_TIMEVAL(t->t_stop),
		  -1.0,       /* report_delta ???  */
		  logfname, 
		  0 );

  return res;
}


int xml_hpm(void *ptr, taskdata_t *t, region_t *reg) {
  int i, nc;
  int res=0;
  double gflops=0.0;
  
#ifdef HAVE_PAPI
  
  nc=0;
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    if( (papi_events[i].name[0]) )
      nc++;
  }
  
  gflops = ipm_papi_gflops(reg->ctr, reg->wtime);
  
  res += ipm_printf(ptr, "<hpm api=\"PAPI\" ncounter=\"%d\" eventset=\"0\" gflop=\"%.5e\">\n", 
		    nc, gflops);
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    if( !(papi_events[i].name[0]) )
      continue;
    
    res += ipm_printf(ptr, "<counter name=\"%s\" > %lld </counter>\n",
		      papi_events[i].name, reg->ctr[i]);
  }
  res += ipm_printf(ptr, "</hpm>\n");
#endif /* HAVE_PAPI */

  return res;
}


int xml_func(void *ptr, taskdata_t *t, region_t *reg, ipm_hent_t *htab, int actv) {
  int nkey;
  scanspec_t spec;
  scanstats_t stats;
  region_t *tmp;
  int id, res=0;

  /* if needed, walk up region tree and assign 
     same id as closest parent has */
  if( internal2xml[reg->id]<0 ) {
    tmp=reg->parent;
    while(tmp) {
      id=internal2xml[tmp->id];
      if( id>=0 ) {
	internal2xml[reg->id]=id;
	break;
      }
      tmp=tmp->parent;
    }
  }

  scanspec_unrestrict_all(&spec);
  scanspec_restrict_activity(&spec, actv, actv);
  scanspec_restrict_region(&spec, reg->id, reg->id);

  nkey = htable_scan( htab, &stats, spec );
  if( nkey>0 ) {
    res += ipm_printf(ptr, "<func name=\"%s\" count=\"%"IPM_COUNT_TYPEF"\" bytes=\"%.4e\" > %.4e </func>\n", 
		      ipm_calltable[actv].name, stats.hent.count, 
		      stats.bytesum, stats.hent.t_tot);
  }

  if( !(reg->flags)&FLAG_PRINT_EXCLUSIVE) {
    /* also print the func entries for all sub-regions, recursively.
       -> this makes the listed <func> entries inclusive */
    tmp=reg->child;
    while(tmp) {
      res += xml_func(ptr, t, tmp, htab, actv);
      tmp=tmp->next;
    }
  }
  return res;
}



int xml_noregion(void *ptr, taskdata_t *t, region_t *reg, ipm_hent_t *htab) {
  double wtime, utime, stime, mtime;
  region_t noregion, *tmp;
  int i, res=0;
  
  rstack_clear_region(&noregion);
  noregion.id=1;
  noregion.nexecs=reg->nexecs;
  sprintf(noregion.name, "ipm_noregion");
  noregion.flags |= FLAG_PRINT_EXCLUSIVE; 
  noregion.child = reg->child;

  wtime = reg->wtime;
  utime = reg->utime;
  stime = reg->stime;
  mtime = reg->mtime;  

#ifdef HAVE_PAPI
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    noregion.ctr[i]=reg->ctr[i];
  }
#endif
  
  tmp = reg->child;
  while(tmp) {
    wtime -= tmp->wtime;
    utime -= tmp->utime;
    stime -= tmp->stime;
    mtime -= tmp->mtime;
    
#ifdef HAVE_PAPI
    for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
      noregion.ctr[i] -= tmp->ctr[i];
    }
#endif
  
    tmp = tmp->next;
  }

  noregion.wtime=wtime;
  noregion.utime=utime;
  noregion.stime=stime;
  noregion.mtime=mtime;

  res+=xml_region(ptr, t, &noregion, htab);

  return res;
}


int xml_region(void *ptr, taskdata_t *t, region_t *reg, ipm_hent_t *htab) {
  int i, j,  res;
  int offs, range;
  
  res=0;

  /* OLD IPM2 version
  res += ipm_printf(ptr, "<region label=\"%s\" nexits=\"%u\" "
		    "wtime=\"%.5e\" utime=\"%.5e\" stime=\"%.5e\" "
		    "mtime=\"%.5e\" iotime=\"%.5e\" omptime=\"%.5e\" ompidletime=\"%.5e\" >\n", 
		    reg->name, reg->nexecs, 
		    reg->wtime, reg->utime, reg->stime, 
		    reg->mtime, reg->iotime, reg->omptime, reg->ompidletime);
  */

  res += ipm_printf(ptr, "<region label=\"%s\" nexits=\"%u\" "
		    "wtime=\"%.5e\" utime=\"%.5e\" stime=\"%.5e\" "
		    "mtime=\"%.5e\" id=\"%d\" >\n",
		    reg->name, reg->nexecs, reg->wtime, 
		    reg->utime, reg->stime, reg->mtime,
		    internal2xml[reg->id]);
  
  res += xml_modules(ptr, t, reg);

  res += xml_hpm(ptr,t,reg);

  /* print the <func> entries for all modules */
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    offs  = modules[i].ct_offs;
    range = modules[i].ct_range;
    
    if( !(modules[i].name) || range==0 ) 
      continue;
    
    for( j=0; j<range; j++ ) {
      if( !(ipm_calltable[offs+j].name) ) 
	continue;
      
      res += xml_func(ptr, t, reg, htab, offs+j);
    }
  }
  
  /* print child regions, only if we are not 
     restricting to 1st level regions only */
  if( ((t->flags)&FLAG_NESTED_REGIONS) &&
      !(reg->flags&FLAG_PRINT_EXCLUSIVE) && reg->child ) {
    res+=xml_regions(ptr, t, reg, htab);
  }

  res += ipm_printf(ptr, "</region>\n");

  return res;
}



int xml_regions(void *ptr, taskdata_t *t, struct region *reg, ipm_hent_t *htab) {
  struct region *tmp;
  int nreg, res=0;
  int nextid=0;

  /* count child regions */
  nreg=0;
  tmp=reg->child;
  while(tmp) { 
    nreg++;
    tmp=tmp->next;
  }
  if( reg==t->rstack->child ) {
    nreg++; // account for ipm_noregion
  }

  res += ipm_printf(ptr, "<regions n=\"%u\" >\n", nreg);
  
  /* loop over child regions */
  tmp=reg->child;
  while(tmp) { 

    if( (t->flags)&FLAG_NESTED_REGIONS ) {
      /* if printing all regions, the numbers are off
	 by 1 due to ipm_main */
      internal2xml[tmp->id]=(tmp->id)-1;
    }
    else {
      /* 1st level regions are re-numbered sequentially */
      internal2xml[tmp->id]=++nextid;
    }
    
    res += xml_region(ptr, t, tmp, htab);    
    tmp=tmp->next;
  }

  if( reg==t->rstack->child ) {
    res+=xml_noregion(ptr, t, reg, htab);
  }
  
  res += ipm_printf(ptr, "</regions>\n");

  return res;
}


int xml_ptrtable(void *ptr, taskdata_t *t) {
  int i, res=0;

#ifdef HAVE_CUDA
  res += ipm_printf(ptr, "<ptrtable>\n");
  for( i=0; i<CUDA_MAXNUM_KERNELS; i++ ) {
    if( cudaptr[i].ptr ) {
      res+= ipm_printf(ptr, "<ptr addr=\"0x%.16x\">%s</ptr>\n",
		       cudaptr[i].ptr, cudaptr[i].name);
    }
  }
  res += ipm_printf(ptr, "</ptrtable>\n");
#endif /* HAVE_CUDA */

  return res;
}


int xml_hash(void *ptr, taskdata_t *t, ipm_hent_t *htab) {
  int i, res;
  int slct, call, bytes, reg, csite;
  int op, dtype;
  IPM_COUNT_TYPE count;
  IPM_RANK_TYPE orank;
  double tmi, tma, tto;
  int nkey, tid;
  scanstats_t stats;
  char buf[80];


  nkey = 0;

#ifdef HAVE_MPI
  nkey += htable_scan_activity( htab, &stats,
			       MPI_MINID_GLOBAL, MPI_MAXID_GLOBAL);
  count = stats.hent.count;
#endif 

#ifdef HAVE_OMPTRACEPOINTS
  nkey += htable_scan_activity( htab, &stats,
				OMP_MINID_GLOBAL, OMP_MAXID_GLOBAL);
  count += stats.hent.count;
#endif
#ifdef HAVE_POSIXIO
  nkey += htable_scan_activity( htab, &stats,
				POSIXIO_MINID_GLOBAL, POSIXIO_MAXID_GLOBAL);
  count += stats.hent.count;
#endif

#ifdef HAVE_MPIIO
  nkey += htable_scan_activity( htab, &stats,
				MPIIO_MINID_GLOBAL, MPIIO_MAXID_GLOBAL);
  count += stats.hent.count;
#endif /* HAVE_MPIIO */

  res=0;
  res += ipm_printf(ptr, "<hash nlog=\"%"IPM_COUNT_TYPEF"\""
		 " nkey=\"%d\" >\n", stats.hent.count, nkey);
  
  for( i=0; i<MAXSIZE_HASH; i++ )
    {
      if( htab[i].count==0 ) 
        continue;
      
      call  = KEY_GET_ACTIVITY(htab[i].key);

      /* restrict to calls of interest */
      if( 0
#ifdef HAVE_MPI
	 && (call<MPI_MINID_GLOBAL || call>MPI_MAXID_GLOBAL)
#endif 
#ifdef HAVE_OMPTRACEPOINTS
	 && (call<OMP_MINID_GLOBAL || call>OMP_MAXID_GLOBAL)
#endif
#ifdef HAVE_POSIXIO
	 && (call<POSIXIO_MINID_GLOBAL || call>POSIXIO_MAXID_GLOBAL)
#endif
#ifdef HAVE_MPIIO
	 && (call<MPIIO_MINID_GLOBAL || call>MPIIO_MAXID_GLOBAL)
#endif
	  ) 
	continue;
      
      slct  = KEY_GET_SELECT(htab[i].key);

      if( slct==IPM_RESOURCE_BYTES_AND_RANK ) {
	bytes = KEY_GET_BYTES(htab[i].key);
	orank = KEY_GET_RANK(htab[i].key);
      } else {
	bytes = 0;
	orank = 0;
      }
 
      reg   = KEY_GET_REGION(htab[i].key);
      csite = KEY_GET_CALLSITE(htab[i].key);
      tid   = KEY_GET_TID(htab[i].key);
      op    = KEY_GET_OPERATION(htab[i].key);
      dtype = KEY_GET_DATATYPE(htab[i].key);


      if( orank==IPM_RANK_NULL ||
	  orank==IPM_RANK_ANY_SOURCE ||
	  orank==IPM_RANK_ALL ) {
	orank=0;
      } else  {
	if( (print_flags&XML_RELATIVE_RANKS) &&
	    IS_P2P_CALL(call) ) {
	  orank = orank-t->taskid;
	}
      }
      
      tmi = htab[i].t_min;
      tma = htab[i].t_max;
      tto = htab[i].t_tot;
      
      KEY_SPRINT(buf, htab[i].key);

      res += ipm_printf(ptr, "<hent key=\"%s\" call=\"%s\" bytes=\"%d\" "
			"orank=\"%d\" region=\"%d\" callsite=\"%d\" count=\"%" IPM_COUNT_TYPEF "\" tid=\"%d\"",
			buf, ipm_calltable[call].name, 
			bytes, orank, internal2xml[reg], csite, 
			htab[i].count, tid);      
      
      if( slct==IPM_RESOURCE_POINTER ) {
	res += ipm_printf(ptr, " ptr=\"0x%.16x\" stream=\"%d\" ", 
			  KEY_GET_POINTER(htab[i].key),
			  KEY_GET_DATATYPE(htab[i].key));
      }
      
#ifdef IPM_COLLECTIVE_DETAILS
      res += ipm_printf(ptr, " op=\"%s\" dtype=\"%s\"", 
			ipm_mpi_op[op], ipm_mpi_type[dtype]);
#endif


      

      res += ipm_printf(ptr, " >");
      
      res += ipm_printf(ptr, "%.4e %.4e %.4e", tto, tmi, tma);
      res += ipm_printf(ptr, "</hent>\n");
    }
  
  res += ipm_printf(ptr, "</hash>\n");
  return res;
}


int xml_task(void *ptr, taskdata_t *td, ipm_hent_t *htab) 
{
  region_t *ipm_main;
  int i, res;

  for( i=0; i<MAXNUM_REGIONS; i++ ) {
    internal2xml[i]=-1;
  }

  /* td->rstack->child is ipm_main */
  ipm_main = td->rstack->child;
  internal2xml[ipm_main->id]=0;
  
  res=0;
  res += xml_task_header(ptr, td);
  {
    res += xml_job(ptr, td);
    res += xml_host(ptr, td);
    res += xml_perf(ptr, td);
    res += xml_modules(ptr, td, 0);
    res += xml_switch(ptr, td);
    res += xml_cmdline(ptr, td);
    res += xml_env(ptr, td);
    res += xml_ru(ptr, td);
    res += xml_call_mask(ptr, td);
    res += xml_regions(ptr, td, ipm_main, htab);

    if( task.flags&FLAG_LOG_FULL ) {
      res += xml_ptrtable(ptr, td);
      res += xml_hash(ptr, td, htab);
    }
    
    res += xml_internal(ptr, td);
  }
  res += xml_task_footer(ptr, td);

  return res;
}

#ifdef HAVE_CLUSTERING
int xml_taskcopy(void *ptr, procstats_t *stats)
{
  int res=0;
  
  res += ipm_printf(ptr, "<taskcopy mpi_rank=\"%d\" cluster_rank=\"%d\"></taskcopy>\n",
		    stats->myrank, stats->clrank);
  return res;
}
#endif


void report_set_filename() 
{
  struct stat fstat;

  /* see if logdir is available on task 0 */
  if( task.taskid==0 ) {  
    if( task.flags&FLAG_OUTFILE ) {
      strncpy(logfname, task.fname, MAXSIZE_FILENAME);

    } else {
      if( stat(task.logdir, &fstat) ) {
	IPMERR("logdir %s unavailable, using '.'\n", task.logdir);
	sprintf(task.logdir, ".");
      }
      sprintf(logfname, "%s/%s.ipm.xml",
	      task.logdir, task.fname);
    }
  }
  
#ifdef HAVE_MPI
  /* make sure everyone has same filename */
  IPM_BCAST(logfname, MAXSIZE_FILENAME, 
	    MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

  /* FIXME above exposes MPI variables at IPM level, what if no MPI?*/
}


int report_xml_local(unsigned long flags)
{
  FILE *f;
  char buf[80];
  int i, nreg;
  int size;

  print_selector=PRINT_TO_FILE;
  print_flags=flags;
  size=0;

  report_set_filename();

  f=fopen(logfname, "w");
  if(!f) {
    IPMERR("Could not open IPM log file: '%s'\n", logfname);
    return IPM_EOTHER;
  }
  
  size += xml_profile_header(f); 
  fflush(f);

  size += xml_task(f, &task, ipm_htable);
  fflush(f);
  
  size += xml_profile_footer(f); 
  fflush(f);
  
  return IPM_OK;  
}



#ifdef HAVE_MPI 
int report_xml_atroot(unsigned long flags) 
{
  FILE *f;
  char buf[80];
  int i, nreg;
  MPI_Status stat;
  /* other task, hashtable and regions */
  taskdata_t otask;
  ipm_hent_t ohash[MAXSIZE_HASH];
  region_t oregions[MAXNUM_REGIONS];
  region_t *ostack;
  int size;

#ifdef HAVE_CLUSTERING
  procstats_t ostats;
#endif

  print_selector=PRINT_TO_FILE;
  print_flags=flags;
  size=0;

  if( task.taskid==0 ) {
    
    f=fopen(logfname, "w");
    if(!f) {
      IPMERR("Could not open IPM log file: '%s'\n", logfname);
      return IPM_EOTHER;
    }
    
    size += xml_profile_header(f); 
    fflush(f);

    size += xml_task(f, &task, ipm_htable);
    fflush(f);
    
    for( i=1; i<task.ntasks; i++ )
      {
#ifdef HAVE_CLUSTERING
	if( flags&XML_CLUSTERED ) {

	  IPM_RECV( &ostats, sizeof(procstats_t),
		    MPI_BYTE, i, 36, MPI_COMM_WORLD, &stat );
	  
	  /* if rank i is not a new cluster center, we can just 
	     continue with the next rank. if it is, we receive all the
	     information we need to output the task profile */
	  if( ostats.clrank != ostats.myrank ) {
	    size += xml_taskcopy(f, &ostats);
	    continue;
	  }
	}
#endif
	
	IPM_RECV( ohash, sizeof(ipm_hent_t)*MAXSIZE_HASH, 
		  MPI_BYTE, i, 33, MPI_COMM_WORLD, &stat);


#ifdef HAVE_CUDA
	IPM_RECV( &cudaptr, sizeof(cudaptr), MPI_BYTE, i, 39,
		  MPI_COMM_WORLD, &stat );
#endif
	
	IPM_RECV( &otask, sizeof(taskdata_t), MPI_BYTE, i, 34,
		  MPI_COMM_WORLD, &stat );
	
	IPM_RECV( oregions, sizeof(region_t)*MAXNUM_REGIONS, MPI_BYTE,
		  i, 35, MPI_COMM_WORLD, &stat );
	
	ostack = rstack_unpack(MAXNUM_REGIONS, oregions);
	
	assert(ostack);
	assert(ostack->child);

	otask.rstack = ostack;
	
	size += xml_task(f, &otask, ohash);
	
	rstack_cleanup(ostack);
	if( ostack ) IPM_FREE(ostack);
	
	fflush(f);
      }

    size+=xml_profile_footer(f);
    chmod(logfname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    fclose(f);

  } else {
    
    /* all other ranks send their information to rank 0 */

#ifdef HAVE_CLUSTERING
    if( flags&XML_CLUSTERED ) {

      IPM_SEND( &mystats, sizeof(procstats_t),
		MPI_BYTE, 0, 36, MPI_COMM_WORLD );

      /* check if this rank is a cluster center, if not
	 we're done and don't need to send anything more. 
	 If it is a cluster center, we need to send all the other
	 data structures to rank 0 so that it can output the task 
	 profile */
      if( mystats.clrank!=mystats.myrank ) 
	return IPM_OK;
    } 
#endif
    
    IPM_SEND( ipm_htable, sizeof(ipm_hent_t)*MAXSIZE_HASH, 
	      MPI_BYTE, 0, 33, MPI_COMM_WORLD );

#ifdef HAVE_CUDA
    IPM_SEND( &cudaptr, sizeof(cudaptr), MPI_BYTE, 0, 39,
	      MPI_COMM_WORLD );
#endif    
    
    IPM_SEND( &task, sizeof(taskdata_t), MPI_BYTE, 0, 34, 
	      MPI_COMM_WORLD );
    
    memset( oregions, 0, sizeof(region_t)*MAXNUM_REGIONS );
    rstack_pack( ipm_rstack, MAXNUM_REGIONS, oregions);
    
    IPM_SEND( oregions, sizeof(region_t)*MAXNUM_REGIONS, MPI_BYTE, 
	      0, 35, MPI_COMM_WORLD );
  }
  
  return IPM_OK;
}
#endif  /* HAVE_MPI */


#ifdef HAVE_MPI 
int report_xml_mpiio(unsigned long flags) 
{
  FILE *f;
  char *buf;
  int malloc_ok;
  long long int bufsize, temp;
  long long int file_offset;
  int rv, i;

  MPI_File logfile;
  MPI_Info info;

  /*
   * determine required local buffer size by writing to /dev/null 
   */
  f=fopen("/dev/null", "w");
  if(!f) {
    IPMERR("Can not open /dev/null for writing");
    return IPM_EOTHER;
  }

  print_selector=PRINT_TO_FILE;
  print_flags=flags;

  bufsize=0;
  if( task.taskid==0 ) 
    bufsize += xml_profile_header(f); 

#ifdef HAVE_CLUSTERING
  if( flags&XML_CLUSTERED ) {
    if( mystats.clrank == mystats.myrank ) {
      bufsize += xml_task(f, &task, ipm_htable);    
    }  else {
      bufsize += xml_taskcopy(f, &mystats);
    }
  }
#else
  bufsize += xml_task(f, &task, ipm_htable);    

#endif
  
  if( task.taskid==task.ntasks-1) 
    bufsize += xml_profile_footer(f);     

  fclose(f);

  /*    
   * allocate buffers on each rank and then write to the buffer
   */
  buf=(char*)IPM_MALLOC(2*bufsize); 
  if(buf) 
    malloc_ok=1; 
  else 
    malloc_ok=0;

  rv=1;
  IPM_ALLREDUCE(&malloc_ok, &rv, 1, 
		MPI_INT, MPI_LAND, MPI_COMM_WORLD);
  
  if(!rv) {
    IPMERR("Allocating local buffer (%lu bytes) failed\n", 
	   (unsigned long) bufsize);
    if( buf ) IPM_FREE(buf);
    return IPM_EOTHER;
  }


  /*
   * write to allocated buffer
   */
  print_selector=PRINT_TO_BUFFER;
  print_offset=0;
  
  temp=0;
  if( task.taskid==0 ) 
    temp+=xml_profile_header(buf); 
  
#ifdef HAVE_CLUSTERING
  if( flags&XML_CLUSTERED ) {
    if( mystats.clrank == mystats.myrank ) {
      temp += xml_task(buf, &task, ipm_htable);    
    }  else {
      temp += xml_taskcopy(buf, &mystats);
    }
  }
#else
  temp += xml_task(buf, &task, ipm_htable);    
#endif
    
  if( task.taskid==task.ntasks-1) 
    temp+=xml_profile_footer(buf);     


  /* written bytes must match allocated buffer */
  if( temp!=bufsize ) {
    IPMERR("Written #bytes does not match allocated buffer %lld %lld\n", temp, bufsize);
    if( buf ) IPM_FREE(buf);
    return IPM_EOTHER;
  }

  /* 
   * determine global file offsets 
   */
  PMPI_Scan( &bufsize, &file_offset, 1, MPI_LONG_LONG_INT, MPI_SUM, MPI_COMM_WORLD );
  file_offset-=bufsize;

  /*
   * write file in parallel
   */
  PMPI_Info_create(&info);

  PMPI_Info_set(info,"access_style","write_once");
  PMPI_Info_set(info,"collective_buffering","true");
  PMPI_Info_set(info,"file_perm","0644");
  PMPI_Info_set(info,"romio_cb_read","true");
  PMPI_Info_set(info,"cb_align","2");
  PMPI_Info_set(info,"romio_cb_write","true");
  PMPI_Info_set(info,"cb_config_list","*:1");
  PMPI_Info_set(info,"striping_factor","80");
  PMPI_Info_set(info,"IBM_largeblock_io","true");

  PMPI_Barrier(MPI_COMM_WORLD);

  rv = PMPI_File_open( MPI_COMM_WORLD, logfname, 
		       MPI_MODE_WRONLY|MPI_MODE_CREATE, info, &logfile );

  if(rv!=MPI_SUCCESS) {
    IPMERR("Error opening file '%s' using PMPI_File_open()\n", 
	   logfname);
    if( buf ) IPM_FREE(buf);
    return IPM_EOTHER;
  }

  PMPI_Barrier(MPI_COMM_WORLD);
  
  rv = PMPI_File_set_view(logfile, file_offset, 
			  MPI_CHAR, MPI_CHAR, "native", info);

  if(rv!=MPI_SUCCESS) {
    IPMERR("Error executing PMPI_File_set_view()\n");
    if( buf ) IPM_FREE(buf);
    return IPM_EOTHER;
  }

  rv = PMPI_File_write_all(logfile, buf, bufsize, 
			   MPI_CHAR, MPI_STATUS_IGNORE);
  if(rv!=MPI_SUCCESS) {
    IPMERR("Error executing PMPI_File_write_all()\n");
    if( buf ) IPM_FREE(buf);
    return IPM_EOTHER;
  }

  rv = PMPI_File_close( &logfile );
  if(rv!=MPI_SUCCESS) {
    IPMERR("Error executing PMPI_File_close()\n");
    if( buf ) IPM_FREE(buf);
    return IPM_EOTHER;
  }
  
  if( buf ) IPM_FREE(buf);
  chmod(logfname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  return IPM_OK;
}
#endif /* HAVE_MPI */
