
#ifndef MOD_CALLPATH_H_INCLUDED
#define MOD_CALLPATH_H_INCLUDED

#include "ipm_sizes.h"
#include "ipm_modules.h"

/* ---- initialize the module ---- */
int mod_callpath_init(ipm_mod_t* mod, int flags);


int get_callsite_id();



/* ---- callsite / callgraph ----- */

typedef struct callsite 
{
  int  id;
  char *name;
  void *addr;
  unsigned long long int narrivals;
  struct callsite *parent, *next, *child;
} callsite_t;

/* a callgraph is represented by its root node */
typedef callsite_t callgraph_t;


#define INIT_CALLSITE(site_ )	\
  site_->id        = 0;		\
  site_->name      = 0;		\
  site_->addr      = 0;		\
  site_->narrivals = 0;		\
  site_->parent    = 0;		\
  site_->next      = 0;		\
  site_->child     = 0;


#define COPY_CALLSITE(from_, to_)		\
  to_->id        = from_->id;			\
  to_->name      = from_->name;			\
  to_->addr      = from_->addr;			\
  to_->narrivals = from_->narrivals;		\
  to_->parent    = from_->parent;		\
  to_->next      = from_->next;			\
  to_->child     = from_->child;


/* prototype of functions to be called on each node of
   the callgraph by the traverse function */
typedef void*(*cgfunc_t)(callsite_t *site, int level, int flags, void *ptr);

/* traverse the callgraph in DFS order and apply function 
   func to each node */
void callgraph_traverse(callsite_t *graph, callsite_t *stop, 
			cgfunc_t func, void *ptr);

#define IS_NODE(csite_)				\
  ((csite_->addr!=0))

#define IS_LEAF(csite_)					\
  (((csite_->child) && (csite_->child->addr))?0:1)

extern callgraph_t *ipm_callgraph;

typedef struct cs_hent
{
  void *addr;
  char *fname;
  int   offs;
  int   csid;
  int  pcsid; /* parent csid */
} cs_hent_t;

extern cs_hent_t cs_hash[MAXNUM_CALLSITES];

int cs_hash_lookup_addr(void *addr);
int cs_hash_lookup_csid(int csid);

void callgraph_find_by_csid(callgraph_t *g, callsite_t *site);

int callgraph_count_leaves(callgraph_t *g);

int callgraph_count_nodes(callgraph_t *g);


/* flags for invocation of cgfunc from traverse */
#define VISIT_FIRST         1
#define VISIT_BACKTRACK     2


/* ---- callsite table ----- */

typedef struct
{
  int ncs;         /* number of callsites */
  int *csids; /* the IDs */
  void **cstable;  /* the call-stacks */
} cstable_t;

void init_cstable(cstable_t *t, int size);
void clear_cstable(cstable_t *t);

void callgraph_to_cstable(callgraph_t *g, cstable_t *t);

void merge_cstables(cstable_t *t1, cstable_t *t2, int *u);

void ipm_unify_callsite_ids();





#endif /* IPM_CALLSTACK_H_INCLUDED */

