
#include <math.h>

#include <cube.h>
#include "ipm_parse.h"
#include "write_cube.h"

void region_defs(FILE *f, cube_t *cube, job_t *job, 
		 region_t *reg, cubedata_t *cd);
void region_sevs(FILE *f, cube_t *cube, job_t *job, 
		 region_t *reg, int taskid, cubedata_t *cd);


void write_cube_defs(FILE *f, job_t *job, cubedata_t *cd)
{
  cube_t *cube;
  cube_region *reg;
  cube_metric *met1, *met2, *met;
  char buf[200];

  cube = cd->cube = cube_create();
  
  cube_def_mirror(cube, "http://ipm-hpc.sourceforge.net/");
  cube_def_attr(cube, "description", "Converted from an IPM profile");
  
  cd->time = 
    cube_def_met(cube, "Total Time", "", "", "sec", "", "", "", 0);
  
  cd->calls = 
    cube_def_met(cube, "Number of Calls", "", "", "count", "", "", "", 0);
  
  for( std::map<std::string,module_t*>::iterator it = job->modulemap.begin();
       it!=job->modulemap.end(); ++it ) 
    {
      // intermediate metrics for this module
      met1 = cube_def_met(cube, (*it).first.c_str(), 
			  "", "", "sec", "", "", "", cd->time);
      
      met2 = cube_def_met(cube, (*it).first.c_str(), 
			  "", "", "count", "", "", "", cd->calls);
      
      cd->tmetrics[it->second] = met1;
      cd->cmetrics[it->second] = met2;
      
      // metrics for each func
      for( std::map<std::string, func_t*>::iterator fit=job->funcmap.begin();
	   fit!=job->funcmap.end(); fit++ ) {
	if( fit->second->parent == it->second ) 
	  {
	    if( fit->second->active ) {
	      met = cube_def_met(cube, fit->first.c_str(), 
				 "", "", "sec", "", "", "", met1);
	      
	      cd->tmetrics[fit->second] = met;
	      
	      met = cube_def_met(cube, fit->first.c_str(), 
				 "", "", "count", "", "", "", met2);
	      cd->cmetrics[fit->second] = met;

#if 0
	      for( std::map<std::string, subfunc_t*>::iterator sfit=fit->second->subfuncs.begin();
		   sfit!=fit->second->subfuncs.end(); sfit++ ) 
		{
		  met = cube_def_met(cube, sfit->first.c_str(), 
				     "", "", "sec", "", "", "", cd->tmetrics[fit->second]);
		  cd->tmetrics[sfit->second] = met;

		  met = cube_def_met(cube, sfit->first.c_str(), 
				     "", "", "count", "", "", "", cd->cmetrics[fit->second]);
		  cd->cmetrics[sfit->second] = met;
		}
#endif 

	    }
	  }
      }
    }

  region_defs(f, cube, job, &(job->ipm_main), cd);

  sprintf(buf, "%s@%s [%d tasks]",
	  job->username.c_str(),
	  job->machinename.c_str(),
	  job->ntasks);
	  
  cd->machine = cube_def_mach(cube, buf, "");
  cd->threads.resize(job->ntasks);

  for( list<node_t*>::iterator it = job->nodes.begin();
       it!=job->nodes.end(); ++it ) {

    cube_node *node = cube_def_node(cube, (*it)->name.c_str(), cd->machine);
    
    for( list<int>::iterator it2 = (*it)->tasks.begin();
	 it2!=(*it)->tasks.end(); ++it2 ) 
      {
	sprintf(buf, "Task %d\n", (*it2));
	cube_process *proc = cube_def_proc( cube, buf, (*it2), node);
	
	cd->threads[(*it2)] = cube_def_thrd( cube, "Thread", 0, proc );
      }
  }
  
  //
  // create default topologies if none addey by user
  //
  if( job->topologies.empty() ) {
    topospec_t t;
    int nnodes = job->nodes.size();
    int ntasks = job->ntasks;
    int m, n;

    int maxtpn=0;
    for( list<node_t*>::iterator it = job->nodes.begin();
	 it!=job->nodes.end(); ++it ) {
      
      if( maxtpn<(*it)->tasks.size() ) {
	maxtpn = (*it)->tasks.size();
      }
    }
    
    t.x = maxtpn;
    t.y = nnodes;
    t.z = 1;
    job->topologies.push_back(t);

    t.x = nnodes;
    t.y = maxtpn;
    t.z = 1;
    job->topologies.push_back(t);

    if( nnodes>3 ) {
      m = (int)sqrt((double)nnodes);
      n = m;
      
      while( m*n < nnodes ) n++;
      
      t.x = maxtpn;
      t.y = m;
      t.z = n;
      job->topologies.push_back(t);

      t.x = maxtpn;
      t.y = n;
      t.z = m;
      job->topologies.push_back(t);
    }

    
  }
  
  
  int i, j;
  int dim;
  long dimv[3];
  long coordv[3];
  int periodv[3]={0,0,0};
  
  for( list<topospec_t>::iterator it=job->topologies.begin();
       it!=job->topologies.end(); ++it ) 
    {
      if( !(*it).valid ) 
	continue;

      dimv[0]=(*it).z;
      dimv[1]=(*it).y;
      dimv[2]=(*it).x;
      
      if( dimv[0] && dimv[1] )
	dim=2;
      else continue;
      if( dimv[2] )
	dim=3;
      
      cube_cartesian* cart = cube_def_cart(cube, dim, dimv,  periodv);
      
      for( j=0; j<job->ntasks; j++ )
	{
	  /*
	  if( !(data->jobinfo.rankinfo[j].nodename) )
	    continue;
	  */
	  
	  coordv[0]=j/(dimv[2]*dimv[1]);
	  coordv[1]=(j/dimv[2])%dimv[1];
	  coordv[2]=j%dimv[2];
	  
	  cube_def_coords(cube, cart, 
			  cd->threads[j], coordv);      
	}
    }
  
  cube_write_def(cube, f);


  // call region_sevs for each task with ipm_main
  // for a specific region, region_sevs will iterate over
  // all functions and then call itself recursively for all
  // child-regions
  for( int i=0; i<job->ntasks; i++ )
    {
      region_sevs(f, cube, job, &(job->ipm_main), i, cd);
    }

  cube_write_sev_matrix(cube, f);
}



void region_defs(FILE *f, cube_t *cube, job_t *job, region_t *reg, cubedata_t *cd)
{
  cube_cnode *cnode, *parent;
  cube_region *cr;  
  
  if( reg==&(job->ipm_main) ) {
    cr = cube_def_region(cube, "Application", 0, 0, "", "", "");
    cnode = cube_def_cnode_cs(cube, cr, "", 0, 0);
    cd->cnodes[reg]=cnode;
  } else {
    cr = cube_def_region(cube, reg->name.c_str(), 0, 0, "", "", "");
    parent =  cd->cnodes[reg->parent];
    cnode = cube_def_cnode_cs(cube, cr, "", 0, parent);
    cd->cnodes[reg]=cnode;
  }
  
  for( std::list<region_t *>::iterator it=reg->subregions.begin();
       it!=reg->subregions.end(); ++it )
    {
      region_defs(f, cube, job, (*it), cd);
    }
}



void region_sevs(FILE *f, cube_t *cube, job_t *job, 
		 region_t *reg, int taskid, cubedata_t *cd)
{
  cube_cnode *cnode;
  cube_metric *metric;
  cube_thread *thread;
  double val;
  long long count;
  func_t *func;
  module_t *mod;

  // To write the severity entry we need:
  //   1) metric
  //   2) cnode
  //   3) thread 
  //   4) data value

  cnode = cd->cnodes[reg]; // 2) cnode -> DONE
  thread = cd->threads[taskid]; // 3) thread -> DONE
      

  // init module severities to 0.0
  for( std::map<std::string,module_t*>::iterator it = job->modulemap.begin();
       it!=job->modulemap.end(); ++it )  {  
    mod = it->second;
    if( !mod ) continue;
    
    mod->funcsum.time  = 0.0;
    mod->funcsum.count = 0;
  }


  // iterate over all funcs
  for( std::map<std::string, func_t*>::iterator it=job->funcmap.begin();
       it!=job->funcmap.end(); ++it )
    {
      func   = it->second;
      metric = cd->tmetrics[func];  // 1) metric -> DONE
      mod    = (module_t*)func->parent;

      if( !(func->active) )
	  continue;

      if( !func || !metric || !thread || ! mod) {
        fprintf(stderr, "Should not happen!\n");
        continue;
      }

      // 4) value -> DONE
      metric = cd->tmetrics[func];
      val = (job->taskdata[taskid].funcdata)[std::make_pair(reg->id, func->id)].time_x;
      cube_set_sev( cube, metric, cnode, thread, val);
      mod->funcsum.time += val;


      // same for counts...
      metric = cd->cmetrics[func];
      count = (job->taskdata[taskid].funcdata)[std::make_pair(reg->id, func->id)].count;
      cube_set_sev( cube, metric, cnode, thread, (double)count);
      mod->funcsum.count += count;


#if 0
      for( std::map<string,subfunc_t*>::iterator sfit=func->subfuncs.begin();
	   sfit!=func->subfuncs.end(); sfit++ ) 
	{
	  subfunc = sfit->second;
	  metric = cd->tmetrics[subfunc];
	  val = (job->taskdata[taskid].funcdata)[std::make_pair(reg->id, subfunc->id)].time;
	  if( metric ) {
	    cube_set_sev( cube, metric, cnode, thread, val);
	  }

	  func->subfuncsum.time += val;

	  metric = cd->cmetrics[sfit->second];
	  count = (job->taskdata[taskid].funcdata)[std::make_pair(reg->id, sfit->second->id)].count;
	  if( metric ) {
	    cube_set_sev( cube, metric, cnode, thread, (double)count);
	  }
	  
	  func->subfuncsum.count+=count;
	}
#endif 


    }

  // set module severities to sum
  for( std::map<std::string,module_t*>::iterator it = job->modulemap.begin();
       it!=job->modulemap.end(); ++it )  {  
    mod = it->second;
    if( !mod ) continue;

    metric = cd->tmetrics[mod];
    val = mod->funcsum.time;
    cube_set_sev( cube, metric, cnode, thread, val);    

    metric = cd->cmetrics[mod];
    count = mod->funcsum.count;
    cube_set_sev( cube, metric, cnode, thread, (double)count);    
  }

  // overall time in app...
  val = ((job->taskdata)[taskid].regdata)[reg].wtime_x;
  cube_set_sev( cube, cd->time, cnode, thread, val);    

  // overall counts...
  val =0.0;
  for( std::map<std::string,module_t*>::iterator it = job->modulemap.begin();
       it!=job->modulemap.end(); ++it )  {  
    mod = it->second;
    if( !mod ) continue;

    val += (double)mod->funcsum.count;
  }
  
  cube_set_sev( cube, cd->calls, cnode, thread, val);    

  // recursive call for the child regions...
  for( std::list<region_t *>::iterator it=reg->subregions.begin();
       it!=reg->subregions.end(); ++it ) 
    {
      region_sevs(f, cube, job, (*it), taskid, cd);
    }
}
