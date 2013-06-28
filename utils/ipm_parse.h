
#ifndef IPM_PARSE_H_INCLUDED
#define IPM_PARSE_H_INCLUDED

#include <string>
#include <list>
#include <vector>
#include <map>

#include <stdio.h>
#include <time.h>
#include <mxml.h>

#include "report.h"

using std::string;
using std::list;
using std::vector;
using std::map;

#define IPMP_OK     0
#define IPMP_ERR    99

typedef struct topospec { 
  bool valid;
  unsigned x, y, z; 
  
  topospec() 
  {
    x=y=z=0;
    valid=1;
  }
} topospec_t;


// base class for func and module so 
// that both have an unique id
struct mbase
{
  struct mbase *parent;
  static int nextid;
  int id;
};

typedef struct funcdata
{
  // inclusive data
  double    time;
  long long count;

  // exclusive data
  double    time_x;
  long long count_x;

  funcdata() {
    time=0.0;   count=0;
    time_x=0.0; count_x=0;
  }
} funcdata_t;


struct func;
typedef struct module : public mbase
{
  string name;
  list<func*> funcs;
  
  // summed over all funcs in this module
  funcdata_t funcsum;
  
  module(string s) {
    id=nextid++; 
    name=s;
    parent=0;
  }
} module_t;

typedef struct func : public mbase
{
  string name;

  // calltable-id for use in the print_banner function
  int cid; 

  // not all funcs contained in the calltable actually 
  // have to appear in the  <func> entries. 
  // if active==1 there is at least 1 func entry
  bool active;

  func(string s) {
    id=nextid++;
    name=s; parent=0;
  }
  func(string s, module_t *m) {
    id=nextid++;
    name=s; 
    parent=m;
  }
} func_t;


typedef struct region
{
  int level, id, xmlid;
  static int nextid;
  
  struct region *parent;
  string name;

  // list of child regions
  list<struct region*> subregions;

  region() { level=0; id=nextid++; parent=0; xmlid=0; }
  region(region* p) { level=0; id=nextid++; parent=p; xmlid=0; }

  struct region* find_by_xmlid(int id) {
    list<struct region*>::iterator it;
    for( it=subregions.begin(); it!=subregions.end(); it++ ) {
      if( (*it)->xmlid==id ) {
	return (*it);
      }
    }
    for( it=subregions.begin(); it!=subregions.end(); it++ ) {
      return (*it)->find_by_xmlid(id);
    }
    return 0;
  }
  
  void print(FILE *f);
} region_t;


typedef struct node
{
  string name;

  double energy;

  // the tasks executed on this node
  list<int> tasks;

} node_t;


// per-region data (and per task as well)
typedef struct regdata
{
  // inclusive wallclock (from IPM XML)
  double wtime;
  
  // exclusiv wallclock time (computed)
  double wtime_x;

  funcdata_t funcsum;
  
  regdata() {
    wtime=0.0;
    wtime_x=0.0;
  }
} regdata_t;

typedef struct 
{
  // hostname for this task;
  string hostname;

  node_t *node;

  double procmem;
  
  double gflop;

  double energy; // in joules

  // wallclock execution time (inclusive) for each region
  // and whole application (at &(job->ipm_main))
  map<region_t*, regdata_t> regdata;

  // per-func data is kept in a map that is indexed
  // by a pair of indices. The first index is the id of the region
  // the second id is for the func
  map<std::pair<int,int>, funcdata_t> funcdata;
} taskdata_t;

enum outform_t { TERSE, FULL, HTML, CUBE, SUMMARY };

typedef struct job
{
  FILE  *infile, *outfile;
  string inname,  outname;
  
  outform_t outform;
  bool quiet;

  // number of <task> entries in XML
  int  ntasks;

  // XML node and id of current task or 0 
  mxml_node_t *ctask;
  int         taskid;

  list<topospec_t> topologies;

  list<node_t*> nodes;

  string machinename;
  string hostname;    // hostname of log-writer
  string username;
  string cmdline;
  string realpath;

  struct timeval start, final;
  
  // per-task data indexed by task id
  vector<taskdata_t> taskdata;

  map<string,module_t*>  modulemap;
  map<string,func_t*>    funcmap;

  // region representing the whole application
  region_t ipm_main;

  job() {
    infile=outfile=0;
    ntasks=0;
    ctask=0;
    outform=TERSE;
    ipm_main.name="ipm_main";
    ipm_main.parent=0;
    
    start.tv_sec=0; start.tv_usec=0;
    final.tv_sec=0; final.tv_usec=0;
  }

  void gstats_procmem(gstats_t *g); 
  void gstats_gflop(gstats_t *g); 
  void gstats_energy(gstats_t *g); 

  void gstats_region(banner_t *b, region_t *reg, regstats_t *rstats); 
  void banner_set_flags(banner_t *b);
  void banner_set_calltable(banner_t *b);
  void banner_set_regions(banner_t *b, region_t *reg);
  void banner_set_basics(banner_t *b);


  void compute_xdata();
  void compute_xdata_region( region_t *reg, int taskid);

  region_t* find_region_by_xmlid(int id);

  void dump(FILE*f);
} job_t;

int IPM_DIAG(job_t *job, const char* format, ...);

#endif /* IPM_PARSE_H_INCLUDED */

