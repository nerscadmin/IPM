
#ifndef REGSTACK_H_INCLUDED
#define REGSTACK_H_INCLUDED

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_sizes.h"


int rstack_init(int flags);

#define FLAG_PRINT_EXCLUSIVE    1

typedef struct region
{
  struct region *parent, *next, *child;  
  struct region *self; /* needed for packing */

  int id;
  int flags;
  unsigned nexecs;

  /* accumulated time for this region */
  /* mtime retained for backwards compatibility */
  double wtime, utime, stime; 
  double mtime;  
  
  /* enter timestamps */
  double wtime_e, utime_e, stime_e; 
  double mtime_e;
  
  char name[MAXSIZE_REGLABEL+1];

  /* a module can store data with a region */
  void *moddata[MAXNUM_MODULES];

#ifdef HAVE_PAPI
  /* accumulated values for this region */
  long long ctr[MAXNUM_PAPI_EVENTS];
  
  /* snapshot of values when entering region */
  long long ctr_e[MAXNUM_PAPI_EVENTS]; 
  
  /* counter values accumulated while in IPM code */
  long long ctr_ipm[MAXNUM_PAPI_EVENTS]; 
#endif 
} region_t;


typedef struct 
{
  int id;
  region_t *reg;
  char *name;
} regid_t;


void rstack_init_region(struct region *r, char *s);
void rstack_clear_region(struct region *r);


extern struct region *ipm_rstack;
extern struct region *ipm_rstackptr;

/* this is a region representing the execution of the
   application from the start to the point of writing 
   the job log. This is either at the program end or 
   when triggered by snapshotting */
extern struct region ipm_app;


/* 
   op = -1  to exit  a region
   op =  1  to enter a region
   
   maintains the region stack and updates 
   performance data using the ipm_region_begin()
   ipm_region_end() calls, tag is the
   name of the region
*/
void ipm_region(int op, char *tag);

void ipm_region_begin(struct region *r);
void ipm_region_end(struct region *r);


/* prototype of functions to be called on each node of
   the region stack by the traverse function */
typedef void*(*rsfunc_t)(region_t *reg, unsigned level, int flags, void *ptr);

/* DFS traversal of stack */
void traverse_rstack( region_t *stack, region_t *stop,
		      rsfunc_t func, void *ptr);


int rstack_cleanup(region_t *rstack);


int rstack_count_all_regions(region_t *rstack);
int rstack_count_l1_regions(region_t *rstack);

void rstack_print(region_t *rstack, FILE *f);

void rstack_pack(region_t* rstack, int nreg, region_t *list);
region_t* rstack_unpack(int nreg, region_t *list);

region_t* rstack_find_region_by_id(region_t *rstack, int id);
region_t* rstack_find_region_by_name(region_t *rstack, char *name);

#endif /* IPM_RSTACK_H_INCLUDED */
