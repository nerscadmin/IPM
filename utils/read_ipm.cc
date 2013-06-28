
#include <stdio.h>
#include <mxml.h>

#include "ipm_parse.h"

void parse_calltable(mxml_node_t *node, job_t *job);
void parse_task(mxml_node_t *node, job_t *job);
void parse_job(mxml_node_t *node, job_t *job);
void parse_perf(mxml_node_t *node, job_t *job);
void parse_host(mxml_node_t *node, job_t *job);
void parse_cmdline(mxml_node_t *node, job_t *job);
void parse_regions(mxml_node_t *node, job_t *job, region_t *reg);
void parse_region(mxml_node_t *node, job_t *job, region_t *reg);
void parse_func(mxml_node_t *node, job_t *job, region_t *reg);
void parse_hash(mxml_node_t *node, job_t *job);
void parse_hent(mxml_node_t *node, job_t *job);
void parse_ptrtable(mxml_node_t *node, job_t *job);
void parse_ptr(mxml_node_t *node, job_t *job);
void parse_modules(mxml_node_t *node, job_t *job);
void parse_module_energy(mxml_node_t *node, job_t *job);

std::map<std::string, std::string> ptrtable;


//
// this function is called for each XML element in the stream
//
void sax_parse(mxml_node_t *node,
	       mxml_sax_event_t event,
	       void *data)
{
  job_t *job = (job_t*)data;
  mxml_node_t *tmp;
  
  if(event == MXML_SAX_ELEMENT_OPEN) {
    char *name = node->value.element.name;
    
    if(!strcmp(name, "task")) 
      {
	// is this the first task we encounter?
	if( !(job->ctask) ) { 	  
	  
	  // yes -> get some basic info about the job
	  job->ntasks = atoi(mxmlElementGetAttr(node, "mpi_size"));
	  job->username = mxmlElementGetAttr(node, "username");

	  for( list<topospec_t>::iterator it=job->topologies.begin();
	       it!=job->topologies.end(); ++it ) 
	    {
	      if( (*it).x * (*it).y * (*it).z < job->ntasks ) {

		IPM_DIAG(job, "Topology [%dx%dx%d] is not large enough"
			 " for this job with %d tasks, ignorning it...\n", 
			(*it).x, (*it).y, (*it).z, job->ntasks);

		(*it).valid=false;
	      }
	    }
	  
	  job->taskdata.resize(job->ntasks);
	  
	  // find the root node and parse calltable
	  tmp=node; while(tmp->parent) tmp=tmp->parent; 
	  parse_calltable(tmp, job);

	} else {
	
	  // delete the in-memory structure for the old task not
	  // needed any more...
	  mxmlDelete(job->ctask);
	} 

	job->ctask=node;
	job->taskid = atoi(mxmlElementGetAttr(node, "mpi_rank"));
	
	IPM_DIAG(job, "\rParsing task %5d of %5d (%5.2f%%)", 
		 job->taskid, job->ntasks, 
		 100.0*(double)job->taskid/(double)(job->ntasks));
      }
  }

  if (event == MXML_SAX_ELEMENT_CLOSE) {
    char *name = node->value.element.name;

    if(!strcmp(name, "task")) {
      // at this point we have a complete <task></task> 
      // entity in memory, so parse it...
      parse_task(node, job);
    }
  }
  else {
    mxmlRetain(node);
  }
}



int read_ipm(job_t *job) 
{   
  mxmlSAXLoadFile(NULL, job->infile,
		  MXML_TEXT_CALLBACK,
		  sax_parse, job);
  
  IPM_DIAG(job, "\rParsing done.                               \n");

  return IPMP_OK;
}


void parse_calltable(mxml_node_t *node, job_t *job) {
  mxml_node_t *tmp;
  mxml_node_t *sec, *ent;
  char *modname, *fname;
  module_t *mod;
  func_t *func;
  
  tmp = mxmlFindElement(node, node, "calltable",
			NULL, NULL,
			MXML_DESCEND);
  
  if( !tmp ) {
    // The calltable is not included in older profiles. For these
    // cases we assume we only have a single implicit" MPI" module;
    // func_t entries will later be added when parsing the actual
    // <func> entries in this case.
    job->modulemap["MPI"] = new module_t("MPI");
    return;
  }

  // iterate over the sections...
  sec = tmp->child;
  while( sec ) {
    modname = (char*)mxmlElementGetAttr(sec, "module");
    if( modname ) {
      mod = job->modulemap[modname] = new module_t(modname);
      
      // iterate over all entries...
      ent = sec->child;
      while( ent ) {
	fname = (char*)mxmlElementGetAttr(ent, "name");
	
	if( fname ) {
	  func = new func_t(fname, mod);
	  job->funcmap[fname] = func;
	  mod->funcs.push_back(func);
	}
	ent = ent->next;
      }
    }
    sec = sec->next;
  }
}


void parse_task(mxml_node_t *node, job_t *job) {
  mxml_node_t *tmp;

  // <job>  
  tmp = mxmlFindElement(node, node, "job", NULL, NULL,
			MXML_DESCEND);
  if( tmp ) parse_job(tmp, job );
  

  // <regions>  
  tmp = mxmlFindElement(node, node, "regions", NULL, NULL,
			MXML_DESCEND);
  if( tmp ) {
    parse_regions(tmp, job, &(job->ipm_main) );
  } else {
    IPM_DIAG(job, "No <regions> entry found for task %d\n", 
	     job->taskid);
  }
  

  // <perf>  
  tmp = mxmlFindElement(node, node, "perf", NULL, NULL,
			MXML_DESCEND);
  if( tmp ) {
    parse_perf(tmp, job );
  } else {
    IPM_DIAG(job, "No <perf> entry found for task %d\n", 
	     job->taskid);
  } 


  // <cmdline>
  tmp = mxmlFindElement(node, node, "cmdline", NULL, NULL,
			MXML_DESCEND);
  if( tmp ) {
    parse_cmdline(tmp, job );
  } else {
    IPM_DIAG(job, "No <cmdline> entry found for task %d\n", 
	     job->taskid);
  } 


  // <host>  
  tmp = mxmlFindElement(node, node, "host", NULL, NULL,
			MXML_DESCEND);
  if( tmp ) {
    parse_host(tmp, job );
  } else {
    IPM_DIAG(job, "No <host> entry found for task %d\n", 
	     job->taskid);
  }

  // <modules>
  tmp = mxmlFindElement(node, node, "modules", NULL, NULL,
			MXML_DESCEND);
  if( tmp ) parse_modules(tmp, job);

  
  // <ptrtable>  
  tmp = mxmlFindElement(node, node, "ptrtable", NULL, NULL,
			MXML_DESCEND);
  if( tmp ) parse_ptrtable(tmp, job);


  // <hash>  
  tmp = mxmlFindElement(node, node, "hash", NULL, NULL,
			MXML_DESCEND);
  if( tmp ) parse_hash(tmp, job);
}


void parse_regions(mxml_node_t *node, job_t *job, region_t *reg) {
  mxml_node_t *tmp;

  tmp=node?node->child:0;
  while(tmp) {
    
    if( tmp->type==MXML_ELEMENT && 
	!strcmp(tmp->value.element.name, "region") ) 
      {
	parse_region(tmp, job, reg);
      }
    
    tmp=tmp->next;
  }
}


void parse_region(mxml_node_t *node, job_t *job, region_t *parent) {
  mxml_node_t *tmp;
  region_t *reg;
  double time;
  char *str;
  int xmlid;
  taskdata_t &td = (job->taskdata[job->taskid]);

  str = (char*)mxmlElementGetAttr(node, "label");

  // search if region already added 
  // (from parsing a previous task)
  reg=0;
  for( std::list<struct region*>::iterator it=parent->subregions.begin();
       it!=parent->subregions.end(); ++it ) {
    if( (*it) && (*it)->name == str ) {
      reg=*it;
    }
  }

  if( !reg ) {
    reg = new region(parent);
    reg->level = (parent->level)+1;
    reg->name = str;

    str = (char*)mxmlElementGetAttr(node, "id");
    if( str )
      xmlid = atoi(str);
    else 
      xmlid = 0;
   
    reg->xmlid=xmlid;

    parent->subregions.push_back(reg);
  } 
  
  str = (char*)mxmlElementGetAttr(node, "wtime");
  time = atof(str);
  td.regdata[reg].wtime=time;

  tmp=node->child;
  while(tmp) {
    if( tmp->type==MXML_ELEMENT ) {
      if( !strcmp(tmp->value.element.name, "func") ) {
	parse_func(tmp, job, reg);
      }
      if( !strcmp(tmp->value.element.name, "regions") ) {
	parse_regions(tmp, job, reg);
      }
    }
    tmp=tmp->next;    
  }
}


void parse_func(mxml_node_t *node, job_t *job, region_t *reg) {
  module_t *mod;
  func_t *func;
  long long count;
  double time;
  char *str;
  taskdata_t &td = (job->taskdata[job->taskid]);
  
  str = (char*)mxmlElementGetAttr(node, "name");
  if( node->child && node->child->type==MXML_TEXT) {
    time = 0.0;
    time = atof(node->child->value.text.string);
  }
  
  // funcmap is either already populated from parsing the calltable,
  // or it is populated just-in-time when parsing the actual <func>
  // entries. In the latter case, we don't know which module the funcs
  // come from, so assume it's from MPI
  func = job->funcmap[str];
  if( !func ) {
    mod = job->modulemap["MPI"];
    if( !mod ) {
      mod = job->modulemap["MPI"] = new module((std::string)"MPI");
    }
    func = new func_t(str, mod);
    job->funcmap[str] = func;
    mod->funcs.push_back(func);
  }
  mod = (module_t*)func->parent;

  func->active=true;
  td.funcdata[std::make_pair(reg->id, func->id)].time+=time;  
  if( str[0]!='@') {
    td.funcdata[std::make_pair(reg->id, mod->id)].time+=time;
    if( reg->parent==&(job->ipm_main) ) {
      td.funcdata[std::make_pair(job->ipm_main.id, mod->id)].time+=time;
    }
  }

  str = (char*)mxmlElementGetAttr(node, "count");
  count = atoll(str);
  
  td.funcdata[std::make_pair(reg->id, func->id)].count+=count;
  if( str[0]!='@') {
    td.funcdata[std::make_pair(reg->id, mod->id)].count+=count;
    if( reg->parent==&(job->ipm_main) ) {
      td.funcdata[std::make_pair(job->ipm_main.id, mod->id)].count+=count;
    }
  }

  td.regdata[reg].funcsum.time+=time;
  td.regdata[reg].funcsum.count+=count;
}


void parse_perf(mxml_node_t *node, job_t *job) {
  char *str;
  double time=0.0, procmem=0.0;
  double gflop=0.0;
  taskdata_t &td = (job->taskdata[job->taskid]);

  str = (char*)mxmlElementGetAttr(node, "wtime");
  if(str) { time = atof(str); }
  td.regdata[&(job->ipm_main)].wtime=time;

  str = (char*)mxmlElementGetAttr(node, "gbyte");
  if(str) { procmem = atof(str); }
  td.procmem=procmem;

  str = (char*)mxmlElementGetAttr(node, "gflop");
  if(str) { gflop = atof(str); }
  td.gflop = gflop;
}


void parse_host(mxml_node_t *node, job_t *job) {
  char *str;
  node_t *nde;

  // the <host> entry also contains the "mach_name" attribute
  // parse it if not yet set...
  if( job->machinename.empty() ) {
    str = (char*)mxmlElementGetAttr(node, "mach_name");
    job->machinename = str;
  }

  str=0;
  if( node->child && node->child->type==MXML_TEXT) {
    str = node->child->value.text.string;
    job->taskdata[job->taskid].hostname=str;

    // if hostname for job not set yet, use the first host encountered...
    if( job->hostname.empty() )
      job->hostname=str;
  } else  {
    return;
  }

  // search from back since that's where we find the existing node 
  // most likely...
  nde=0; 
  for( list<node_t*>::reverse_iterator it = job->nodes.rbegin();
       it!=job->nodes.rend(); ++it ) {
    if( (*it)->name == str ) {
      nde = (*it);
    }
  }

  if( !nde ) {
    nde = new node_t();
    nde->name = str;
    job->nodes.push_back(nde);
  }

  nde->tasks.push_back(job->taskid);

  job->taskdata[job->taskid].node=nde;
}

void parse_cmdline(mxml_node_t *node, job_t *job) {
  char *str=0;

  if( job->cmdline.empty() ) {
    if( node->child && node->child->type==MXML_TEXT) {
      str = node->child->value.text.string;
      if( str ) job->cmdline = str;
    }
    
    str = (char*)mxmlElementGetAttr(node, "realpath");
    if( str ) job->realpath = str;
  }
}


void parse_hash(mxml_node_t *node, job_t *job) {
  mxml_node_t *tmp;

  tmp=node->child;
  while(tmp) {
    if( tmp->type==MXML_ELEMENT && 
	!strcmp(tmp->value.element.name, "hent") ) 
      {
	parse_hent(tmp, job);
      }
    tmp=tmp->next;
  }
}

void parse_hent(mxml_node_t *node, job_t *job) {
  char *call, *ptr, *str;
  node_t *nde;
  func_t *func;
  string name;
  int xmlid;
  int count;
  taskdata_t &td = (job->taskdata[job->taskid]);
  region_t *reg;
  mxml_node_t *tmp;
  double ttot, tmin, tmax;
  
  str = (char*)mxmlElementGetAttr(node, "count");
  count=atoi(str);

  ttot=0.0; tmin=0.0; tmax=0.0;
  tmp=node->child;
  if( tmp && tmp->type==MXML_TEXT ) {
    ttot = atof(tmp->value.text.string);
    tmp = tmp->next;
  }
  if( tmp && tmp->type==MXML_TEXT ) {
    tmin = atof(tmp->value.text.string);
    tmp=tmp->next;
  }
  if( tmp && tmp->type==MXML_TEXT ) {
    tmax = atof(tmp->value.text.string);
  }

  call = (char*)mxmlElementGetAttr(node, "call");

  if( !strncmp("@CUDA_EXEC_STRM", call, 15) ) {
    ptr = (char*)mxmlElementGetAttr(node, "ptr");

    func = job->funcmap[call];
    if( !func ) {
      IPM_DIAG(job, "No func found for '%s'!\n", call);
      return;
    }
    name = ptrtable[ptr];

    if( name.empty() ) {
      IPM_DIAG(job, "Not found in ptrtable: %p\n", ptr);
      return;
    } 

    str = (char*)mxmlElementGetAttr(node, "region");
    xmlid=atoi(str);

    reg = job->find_region_by_xmlid(xmlid);
    if( !reg ) {
      IPM_DIAG(job, "Could not find region for xml id %d\n", xmlid);
      return;
    }
  }
}


void parse_ptrtable(mxml_node_t *node, job_t *job) {
  mxml_node_t *tmp;
  
  tmp=node->child;
  while(tmp) {
    if( tmp->type==MXML_ELEMENT && 
	!strcmp(tmp->value.element.name, "ptr") ) 
      {
	parse_ptr(tmp, job);
      }
    tmp=tmp->next;
  }
}

void parse_ptr(mxml_node_t *node, job_t *job) {
  char *str;
  char *name;

  str = (char*)mxmlElementGetAttr(node, "addr");
  if( node->child && node->child->type==MXML_TEXT) {
    name = node->child->value.text.string;
    ptrtable[str] = name;
  }
}

void parse_modules(mxml_node_t *node, job_t *job) {
  char *str;
  mxml_node_t *tmp;

  tmp=node->child;
  while(tmp) {
    if( tmp->type==MXML_ELEMENT && 
	!strcmp(tmp->value.element.name, "module") ) 
      {
	str = (char*)mxmlElementGetAttr(tmp, "name");
	
	if( !strcmp(str, "Energy") ) { 
	  if( !(job->modulemap["ENERGY"]) ) {
	    job->modulemap["ENERGY"] = new module_t("ENERGY");
	  }
	  parse_module_energy(tmp, job);
	}
      }
    tmp=tmp->next;
  }
}

void parse_module_energy(mxml_node_t *node, job_t *job) {
  char *str;
  char *name;
  double en;

  taskdata_t &td = (job->taskdata[job->taskid]);

  str = (char*)mxmlElementGetAttr(node, "consumed");
  en = atof(str);

  td.node->energy = en;
}


void parse_job(mxml_node_t *node, job_t *job) {
  long start, final;
  char *str;

  if( job->start.tv_sec==0 || job->final.tv_sec==0 ) {
    str = (char*)mxmlElementGetAttr(node, "start");
    start = atol(str);

    str = (char*)mxmlElementGetAttr(node, "final");
    final = atol(str);
    
    job->start.tv_sec = start;
    job->final.tv_sec = final;
  }
}
