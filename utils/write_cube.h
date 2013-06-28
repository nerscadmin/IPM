
#ifndef IPM_CUBE_H_INCLUDED
#define IPM_CUBE_H_INCLUDED

#include <vector>
#include <map>
#include <list>
#include <cube.h>

#include "ipm_parse.h"

using std::map;
using std::list;
using std::vector;

typedef struct 
{
  cube_t *cube;
  cube_machine *machine;

  // root metrics
  cube_metric *time;
  cube_metric *calls;

  // metrics for each func
  std::map<mbase*, cube_metric*> tmetrics;
  std::map<mbase*, cube_metric*> cmetrics;

  // cnodes for each region
  std::map<region_t*, cube_cnode*> cnodes;

  // thread for each rank
  std::vector<cube_thread*> threads;

} cubedata_t;


void write_cube_defs(FILE *f, job_t *job, cubedata_t *cd);

#endif /* IPM_CUBE_H_INCLUDED */
