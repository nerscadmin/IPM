
#include <float.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>



#include "report.h"
#include "ipm_parse.h"
#include "write_cube.h"
#include "write_html.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)<(b)?(b):(a))

// static variables 
int region::nextid=0;
int mbase::nextid=0;

banner_t banner;

void usage(int argc, char *argv[]);
int getopts(int argc, char *argv[], job_t *job);
int read_ipm(job_t *job);

int main( int argc, char *argv[])
{
  job_t job;
  struct stat st;
  
  if(getopts(argc, argv, &job)!=IPMP_OK) {
    usage(argc, argv);
    return 1;
  }

  job.infile = (job.inname.empty())?stdin:fopen(job.inname.c_str(), "r");
  stat(job.inname.c_str(),&st);
  if(S_ISDIR(st.st_mode)) {
    fprintf(stderr, "Input '%s' is a directory not a file.\n", 
	    job.inname.c_str());
    usage(argc, argv);
    return 1;
  }
  if( !job.infile ) {
    fprintf(stderr, "Can't open input file '%s'\n", 
	    job.inname.c_str());
    usage(argc, argv);
    return 1;
  }

  job.outfile = (job.outname.empty())?stdout:fopen(job.outname.c_str(), "w");
  if( !job.outfile )  {
    fprintf(stderr, "Can't open output file '%s'\n", 
	    job.outname.c_str());
    usage(argc, argv);
    return 1;
  }

  read_ipm(&job);

  switch(job.outform) {

  case FULL:
    banner.flags |= BANNER_FULL;
    
    job.banner_set_calltable(&banner);
    job.banner_set_regions(&banner, &(job.ipm_main));

    // NOTE: no break here, we fall through to TERSE
    
  case TERSE:

    // cmdline, ntasks, etc...
    job.banner_set_basics(&banner);

    // this will also set per-function data if BANNER_FULL
    job.gstats_region(&banner, &(job.ipm_main), &(banner.app));
    job.banner_set_flags(&banner);
    
    ipm_print_banner(stdout, &banner);
    break;
   
    
  case CUBE:

    // compute exclusive from data read
    job.compute_xdata();
    {
      cubedata_t cdata;
      write_cube_defs(job.outfile, &job, &cdata);
    }
    break;

  case HTML:
    // we also need the banner information in the html report,
    // so just pass a banner_t to the html-out routine

    job.banner_set_calltable(&banner);
    job.banner_set_regions(&banner, &(job.ipm_main));
    banner.flags |= BANNER_FULL;

    job.banner_set_basics(&banner);
    job.gstats_region(&banner, &(job.ipm_main), &(banner.app));
    job.banner_set_flags(&banner);

    write_html(job.outfile, &job, &banner);
    break;

  case SUMMARY:
    job.banner_set_calltable(&banner);
    job.banner_set_regions(&banner, &(job.ipm_main));
    banner.flags |= BANNER_FULL;

    job.banner_set_basics(&banner);
    job.gstats_region(&banner, &(job.ipm_main), &(banner.app));
    job.banner_set_flags(&banner);
    fprintf(job.outfile,"#username cmdline ntasks nhosts nthreads "
	    "sum[wwall wmpi wposix gflops procmem] "
	    "min[wwall wmpi wposix gflops procmem] "
	    "max[wwall wmpi wposix gflops procmem]\n");
    
    fprintf(job.outfile,"%s %s %d %d %d "
	    "%.3e %.3e %.3e %.3e %.3e "
	    "%.3e %.3e %.3e %.3e %.3e "
	    "%.3e %.3e %.3e %.3e %.3e\n",
	 job.username.c_str(),
	 banner.cmdline,
	 banner.ntasks,
	 banner.nhosts,
	 banner.nthreads,

         banner.app.wallt.dsum,
         banner.app.mpi.dsum,
         banner.app.pio.dsum,
         banner.gflop.dsum,
	 banner.procmem.dsum,

         banner.app.wallt.dmin,
         banner.app.mpi.dmin,
         banner.app.pio.dmin,
         banner.gflop.dmin,
	 banner.procmem.dmin,

         banner.app.wallt.dmax,
         banner.app.mpi.dmax,
         banner.app.pio.dmax,
         banner.gflop.dmax,
	 banner.procmem.dmax
    );
    break;

  }
}


void usage(int argc, char *argv[])
{
  fprintf(stderr, 
	  "Usage: %s [options] <IPM XML input> [[-o] <output file>]\n",
	  argv[0]);
  fprintf(stderr, "\nOptions:\n");  
  fprintf(stderr, "  -full    print the full banner\n");
  fprintf(stderr, "  -html    produce HTML report\n");
  fprintf(stderr, "  -summary one-line summary format\n");
  fprintf(stderr, "  -cube    convert to CUBE format\n");
  fprintf(stderr, "  -q       quiet, don't complain\n");
  fprintf(stderr, "\nCUBE options:\n");  
  fprintf(stderr, "  -t n1xn2[xn3][,l1xl2xl3] specify 2D or 3D processor topology\n");
}



void job_t::dump(FILE*f)
{
  module_t *mod;

  for( map<string, module_t*>::iterator it=modulemap.begin();
       it!=modulemap.end(); ++it ) {

    mod = (*it).second; 

    fprintf(f, "MODULE: %s\n", mod->name.c_str());

    for( list<func_t*>::iterator it2 = mod->funcs.begin();
	 it2!=mod->funcs.end(); ++it2 ) 
      {
	fprintf(f, " - '%s'\n", (*it2)->name.c_str());
      }
    fprintf(f, "\n");
  }
}


void job_t::compute_xdata()
{
  for( int i=0; i<ntasks; i++ ) {
    compute_xdata_region(&ipm_main, i);
  }
}

void job_t::compute_xdata_region(region_t *reg, int taskid)
{
  double xtime;
  func_t *func;
  int fid;

  xtime = (taskdata[taskid].regdata)[reg].wtime;

  (taskdata[taskid].regdata)[reg].funcsum.time_x =
    (taskdata[taskid].regdata)[reg].funcsum.time;

  (taskdata[taskid].regdata)[reg].funcsum.count_x =
    (taskdata[taskid].regdata)[reg].funcsum.count;
  
  for( std::list<region_t *>::iterator it=reg->subregions.begin();
       it!=reg->subregions.end(); ++it ) 
    {
      xtime -= (taskdata[taskid].regdata)[*it].wtime;
      
      (taskdata[taskid].regdata)[reg].funcsum.count_x -=
	(taskdata[taskid].regdata)[*it].funcsum.count;

      (taskdata[taskid].regdata)[reg].funcsum.time_x -=
	(taskdata[taskid].regdata)[*it].funcsum.time;
    }
  
  (taskdata[taskid].regdata)[reg].wtime_x = xtime;
  
  
  for( std::map<string,func_t*>::iterator it=funcmap.begin();
       it!=funcmap.end(); ++it ) 
    {
      func = (*it).second;
      
      if( !(func->active) ) 
	continue;
      
      if( reg==(&ipm_main) ) 
	continue;

      fid = func->id;
      funcdata_t&fd = taskdata[taskid].funcdata[std::make_pair(reg->id, fid)];
      fd.time_x = fd.time;
      for( std::list<region_t *>::iterator it2=reg->subregions.begin();
	   it2!=reg->subregions.end(); ++it2 ) 
	{
	  fd.time_x -= (taskdata[taskid].funcdata[std::make_pair((*it2)->id, fid)]).time;
	}
      
    }

  for( std::list<region_t *>::iterator it=reg->subregions.begin();
       it!=reg->subregions.end(); ++it ) 
    {
      compute_xdata_region(*it, taskid);
    }
}


region_t* job_t::find_region_by_xmlid(int id)
{
  region_t *reg=0;

  reg = ipm_main.find_by_xmlid(id);
  
  return reg;
}


// compute global (across the tasks) statistics for procmem
void job_t::gstats_procmem(gstats_t *g)
{
  vector<taskdata_t>::iterator it;

  GSTATS_CLEAR((*g));
  g->dmin = DBL_MAX;
  
  for( it=taskdata.begin(); it!=taskdata.end(); it++ ) 
    {
      g->dmin = MIN(g->dmin, (*it).procmem);
      g->dmax = MAX(g->dmax, (*it).procmem);
      g->dsum += (*it).procmem;
    }
}


void job_t::gstats_gflop(gstats_t *g)
{
  vector<taskdata_t>::iterator it;

  GSTATS_CLEAR((*g));
  g->dmin = DBL_MAX;
  
  for( it=taskdata.begin(); it!=taskdata.end(); it++ ) 
    {
      g->dmin = MIN(g->dmin, (*it).gflop);
      g->dmax = MAX(g->dmax, (*it).gflop);
      g->dsum += (*it).gflop;
    }
}

void job_t::gstats_energy(gstats_t *g)
{
  vector<taskdata_t>::iterator it;

  GSTATS_CLEAR((*g));
  g->dmin = DBL_MAX;

  for( it=taskdata.begin(); it!=taskdata.end(); it++ ) 
  {
      (*it).energy = (it->node->energy)/(double)
		      (it->node->tasks).size();
  }
  
  for( it=taskdata.begin(); it!=taskdata.end(); it++ ) 
    {
      g->dmin = MIN(g->dmin, (*it).energy);
      g->dmax = MAX(g->dmax, (*it).energy);
      g->dsum += (*it).energy;
    }
}

void job_t::gstats_region(banner_t *b, region_t *reg, regstats_t *rstats)
{
  gstats_t *g, *h;
  module_t *mod;
  double time, val;
  IPM_COUNT_TYPE count;
  vector<taskdata_t>::iterator it;

  // wallclock time
  g=&(rstats->wallt);
  GSTATS_CLEAR((*g));
  g->dmin = DBL_MAX;
  g->nmin = IPM_COUNT_MAX;

  for( it=taskdata.begin(); it!=taskdata.end(); it++ ) 
    {
      time = (*it).regdata[reg].wtime;
      g->dmin = MIN(g->dmin, time);
      g->dmax = MAX(g->dmax, time);
      g->dsum += time;
    }

  for( map<string, module_t*>::iterator mit=modulemap.begin();
       mit!=modulemap.end(); ++mit ) 
    {
      mod = (*mit).second; 

      if( mod->name=="MPI" ) {
	g=&(rstats->mpi);
	h=&(rstats->mpip);
      } else if(mod->name=="POSIXIO") {
	g=&(rstats->pio);
	h=&(rstats->piop);
      } else if(mod->name=="OMP") {
	g=&(rstats->omp);
	h=&(rstats->ompp);
      } else if(mod->name=="CUDA") {
	g=&(rstats->cuda);
	h=&(rstats->cudap);
      } else if(mod->name=="CUBLAS") {
	g=&(rstats->cublas);
	h=&(rstats->cublasp);
      } else if(mod->name=="CUFFT") {
	g=&(rstats->cufft);
	h=&(rstats->cufftp);
      } else {
	continue;
      }

      GSTATS_CLEAR((*g));
      g->dmin = DBL_MAX;
      g->nmin = IPM_COUNT_MAX;
      
      GSTATS_CLEAR((*h));
      h->dmin = DBL_MAX;
      h->nmin = IPM_COUNT_MAX;
      
      for( it=taskdata.begin(); it!=taskdata.end(); it++ ) 
	{
	  time = (*it).funcdata[std::make_pair(reg->id, mod->id)].time;
	  count = (*it).funcdata[std::make_pair(reg->id, mod->id)].count;
	  
	  g->dmin = MIN(g->dmin, time);
	  g->dmax = MAX(g->dmax, time);
	  g->dsum += time;
	  g->nmin = MIN(g->nmin, count);
	  g->nmax = MAX(g->nmax, count);
	  g->nsum += count;
	  
	  val = 100.0 * (*it).funcdata[std::make_pair(reg->id, mod->id)].time / 
	    (*it).regdata[reg].wtime;
	  
	  h->dmin = MIN(h->dmin, val);
	  h->dmax = MAX(h->dmax, val);
	  h->dsum += val;
	}
    }

  if((b->flags) & BANNER_FULL) {
    func_t *func;

    for( std::map<string,func_t*>::iterator fit=funcmap.begin();
	 fit!=funcmap.end(); ++fit ) 
      {
	func = (*fit).second;

	if( !(func->active) ) 
	  continue;

	g = &(rstats->fullstats[func->cid]);

	GSTATS_CLEAR((*g));
	g->dmin = DBL_MAX;
	g->nmin = IPM_COUNT_MAX;
	g->activity=func->cid;
		
	for( it=taskdata.begin(); it!=taskdata.end(); it++ ) 
	  {
	    time = (*it).funcdata[std::make_pair(reg->id, func->id)].time;
	    count = (*it).funcdata[std::make_pair(reg->id, func->id)].count;

	    //	    fprintf(stderr, "%d %d %d %f\n", reg->id, func->id, count, time);

	    g->dmin = MIN(g->dmin, time);
	    g->dmax = MAX(g->dmax, time);
	    g->dsum += time;
	    g->nmin = MIN(g->nmin, count);
	    g->nmax = MAX(g->nmax, count);
	    g->nsum += count;
	  }
      }
  }
}


void job_t::banner_set_flags(banner_t *b) 
{
  module_t *mod;
 
  for( map<string, module_t*>::iterator it=modulemap.begin();
       it!=modulemap.end(); ++it ) {

    mod = (*it).second; 

    if( mod->name == "MPI" )     b->flags |= BANNER_HAVE_MPI;
    if( mod->name == "POSIXIO" ) b->flags |= BANNER_HAVE_POSIXIO;
    if( mod->name == "OMP" )     b->flags |= BANNER_HAVE_OMP;
    if( mod->name == "CUDA" )    b->flags |= BANNER_HAVE_CUDA;
    if( mod->name == "CUBLAS" )  b->flags |= BANNER_HAVE_CUBLAS;
    if( mod->name == "CUFFT" )   b->flags |= BANNER_HAVE_CUFFT;
    if( mod->name == "ENERGY" )  b->flags |= BANNER_HAVE_ENERGY;
  }
}

void job_t::banner_set_calltable(banner_t *b) 
{
  func_t *func;
  int idx=0;

  // go through all funcs and store them consecutively in the calltable
  // remember which index each one is in the cid member of func_t 
  for( std::map<string,func_t*>::iterator it=funcmap.begin();
       it!=funcmap.end(); ++it ) 
    {
      func = (*it).second;
      
      if( !(func->active) ) 
	continue;

      b->calltable[idx] = strdup( func->name.c_str() );
      func->cid=idx;
      idx++;

      if( idx>=MAXSIZE_CALLTABLE ) {
	// FIXME: This should probably be IPM_ERR
	IPM_DIAG(this, "Number of <func> entries exceeds MAXSIZE_CALLTABLE\n");
	break;
      }
    }
}


void job_t::banner_set_regions(banner_t *b, region_t *reg) 
{
  int id = reg->id;

  if( id>=MAXNUM_REGIONS ) {
    // FIXME
    IPM_DIAG(this, "Region ID for region '' out of bounds\n", 
	     reg->name.c_str());
    return;
  }
  b->regions[id].valid = 1;
  strcpy(b->regions[id].name, reg->name.c_str());

  if( reg!=&ipm_main ) {
    gstats_region(b, reg, &((b->regions)[id]));
  }
  
  for( std::list<region_t *>::iterator it=reg->subregions.begin();
       it!=reg->subregions.end(); ++it ) 
    {
      banner_set_regions(b, (*it));
    }
  
}

void job_t::banner_set_basics(banner_t *b) 
{
  BANNER_SET_NTASKS(banner, this->ntasks);
  BANNER_SET_NHOSTS(banner, this->nodes.size());
  BANNER_SET_NTHREADS(banner, 1);   /* FIXME */
  
  BANNER_SET_CMDLINE(banner, this->cmdline.c_str());
  BANNER_SET_HOSTNAME(banner, this->hostname.c_str());
  BANNER_SET_TSTART(banner, this->start);
  BANNER_SET_TSTOP(banner, this->final);
  BANNER_SET_NREGIONS(banner, 1);  /* FIXME */
  
  this->gstats_procmem(&(banner.procmem));
  this->gstats_gflop(&(banner.gflop));
  this->gstats_energy(&(banner.energy));
}
  
