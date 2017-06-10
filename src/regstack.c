
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "ipm.h"
#include "ipm_core.h"
#include "ipm_time.h"
#include "regstack.h"
#include "ipm_modules.h"

#ifdef HAVE_PAPI
#include "mod_papi.h"
#endif 

#define VISIT_FIRST      1
#define VISIT_BACKTRACK  2

struct region *ipm_rstack;
struct region *ipm_rstackptr;

struct region ipm_app;

int rstack_init(int flags) {
  ipm_rstack=(region_t*)IPM_MALLOC(sizeof(region_t));

  rstack_init_region(ipm_rstack, "(root)");
  ipm_rstackptr=ipm_rstack;

  rstack_clear_region(&ipm_app);
  sprintf(ipm_app.name, "whole application");
  
  return 0;
}

void rstack_init_region(struct region *r, 
			  char *str) 
{
  int i;

  /* region IDs are numbered consecutively starting with 0 */
  /* the implicit root of the region stack (root) will have ID 0 */
  static int nextid=0; 

  rstack_clear_region(r);
  
  r->id=nextid++;
  strncpy(r->name, str, MAXSIZE_REGLABEL);
  r->name[MAXSIZE_REGLABEL]=0;
}

void rstack_clear_region(struct region *r)
{
  int i;

  r->parent=0;
  r->next=0;
  r->child=0;
  r->name[0]=0;
  r->flags=0;
  
  r->nexecs=0;
  
  r->wtime=0.0;    r->wtime_e=0.0;
  r->utime=0.0;    r->utime_e=0.0;
  r->stime=0.0;    r->stime_e=0.0;
  r->mtime=0.0;    r->mtime_e=0.0;
  
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    r->moddata[i]=0;
  }
  
#ifdef HAVE_PAPI
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    r->ctr[i]=0;
    r->ctr_e[i]=0;
    r->ctr_ipm[i]=0;
  }
#endif
}


void ipm_region_begin(struct region *reg)
{
  int i;

#ifdef HAVE_PAPI
  //long long ctr1[MAXNUM_PAPI_EVENTS];
  long long ctr2[MAXNUM_PAPI_EVENTS];
#endif
  
#ifdef HAVE_PAPI
  //ipm_papi_read(ctr1);
#endif 

  /* fprintf(stderr, "region_begin for reg=%x '%s'\n", reg, reg->name);
   */

  /* update enter stats for ipm_rstackptr */
  reg->wtime_e = ipm_wtime();
  reg->utime_e = ipm_utime();
  reg->stime_e = ipm_stime();
  reg->mtime_e = ipm_mtime();

  for( i=0; i<MAXNUM_MODULES; i++ ) {
    if( modules[i].regfunc ) {
      modules[i].regfunc(&(modules[i]), 1, reg);
    }
  }
  
#ifdef HAVE_PAPI
  ipm_papi_read(ctr2);
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    //reg->ctr_ipm[i] += (ctr2[i]-ctr1[i]);
    reg->ctr_e[i]=ctr2[i];
  }
#endif
}

void ipm_region_end(struct region *reg)
{
  int i;

#ifdef HAVE_PAPI
  long long ctr1[MAXNUM_PAPI_EVENTS];
  //  long long ctr2[MAXNUM_PAPI_EVENTS];
#endif

  /* fprintf(stderr, "region_end for reg=%x name='%s'\n", reg, reg->name);  
   */

#ifdef HAVE_PAPI
  ipm_papi_read(ctr1);
#endif
  
  /* update data for region 'reg' */
  reg->wtime   += (ipm_wtime())  - (reg->wtime_e);
  reg->utime   += (ipm_utime())  - (reg->utime_e);
  reg->stime   += (ipm_stime())  - (reg->stime_e);
  reg->mtime   += (ipm_mtime())  - (reg->mtime_e);
  
  for( i=0; i<MAXNUM_MODULES; i++ ) {
    if( modules[i].regfunc ) {
      modules[i].regfunc(&(modules[i]), -1, reg);
    }
  }

#ifdef HAVE_PAPI
  // ipm_papi_read(ctr2);
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    //reg->ctr_ipm[i] += (ctr2[i]-ctr1[i]);
    reg->ctr[i] += (ctr1[i]-reg->ctr_e[i]);
  }
#endif

}



void ipm_region(int op, char *tag) 
{
  int i;
  struct region *reg;
  
  switch( op ) 
    {
    case -1: /* exit */
      
      ipm_region_end(ipm_rstackptr);
      ipm_rstackptr->nexecs++;

      if( ipm_rstackptr->parent ) {
	ipm_rstackptr=ipm_rstackptr->parent;
      }
      
      break;
      
    case 1: /* enter */
      
      /* find region among child regions of current ipm_rstackptr */
      reg = ipm_rstackptr->child;
      while(reg) {
	if( !(strncmp(reg->name, tag, MAXSIZE_REGLABEL)) ) {
	  ipm_rstackptr=reg;
	  break;
	}
	reg=reg->next;
      }
      if( !reg ) {
	/* region not found, create new one */
	reg=(region_t*)IPM_MALLOC(sizeof(region_t));
	rstack_init_region(reg, tag);
	reg->parent=ipm_rstackptr;
	
	if( !(ipm_rstackptr->child) ) {
	  ipm_rstackptr->child=reg;
	}
	else {
	  ipm_rstackptr=ipm_rstackptr->child;
	  while(ipm_rstackptr->next)
	    ipm_rstackptr=ipm_rstackptr->next;
	  ipm_rstackptr->next=reg;
	}

	ipm_rstackptr=reg;
      }
      ipm_region_begin(ipm_rstackptr);
      
      break;
    }
}

void traverse_rstack(region_t *stack, region_t *stop, 
		     rsfunc_t func, void *ptr)
{
  int levl;
  region_t *node;

  node=stack; levl=0;

  while(node) {
 
    /* ----------------------------------- */
    ptr=func(node, levl, VISIT_FIRST, ptr);
    /* ----------------------------------- */

    /* advance to next node (depth-first) */
    if( node->child ) 	{
      levl++;
      node=node->child;
    }
    else if( node->next ) {
      /* levl unchanged! */
      node=node->next;
    }
    else {
      /* back-tracking... */
      while(1) {
	node=node->parent;
	levl--;

	/* this will also terminate the outer loop */
	if( !node || node==stop ) 
	  break;

	/* ------------------------------------------- */	
	ptr=func(node, levl, VISIT_BACKTRACK, ptr);
	/* ------------------------------------------- */	
	
	if( node->next ) {
	  /* levl unchanged! */
	  node=node->next;
	  break;
	}
      }  
      if( node==stop )
	break;
    }
  }
}  

void* rsfunc_print_region(region_t *reg, unsigned level, int flags, void *ptr) 
{
  int i;
  FILE *f = (FILE*)ptr;

  if( flags==VISIT_BACKTRACK )
    return ptr;

  for( i=0; i<level; i++ )
    fprintf(f, "    ");
  
  fprintf(f, "ID=%u (name='%s') nexec=%u (%.5f %.5f %.5f)\n", 
	  reg->id, reg->name, reg->nexecs,
	  reg->wtime, reg->stime, reg->utime);

  return ptr;
}


void* rsfunc_cleanup(region_t *reg, unsigned level, int flags, void *ptr) 
{
  if( flags==VISIT_FIRST )
    return ptr;

  if( reg->child ) IPM_FREE(reg->child);

  return ptr;
}

void* rsfunc_enum_all_regions(region_t *reg, unsigned level, int flags, void *ptr) 
{
  int *num=(int*)ptr;

  if( flags==VISIT_BACKTRACK )
    return ptr;

  (*num)++;
  //  if( reg->parent ) (*num)++;
  return ptr;
}

void* rsfunc_enum_l1_regions(region_t *reg, unsigned level, int flags, void *ptr) 
{
  int *num=(int*)ptr;

  if( flags==VISIT_BACKTRACK )
    return ptr;

  if( (reg->parent) && (reg->parent->parent) && 
      (reg->parent->parent->parent==0) )
    (*num)++;

  return ptr;
}

void* rsfunc_adjust_ctrs(region_t *reg, unsigned level, int flags, void *ptr) 
{
  int i;
  region_t *tmp;

  if( (reg->child) && flags==VISIT_FIRST )
    return ptr;

#ifdef HAVE_PAPI
  /* we are at a child or backtracking */
  tmp = reg->child; 
  while(tmp) {
    for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
      reg->ctr_ipm[i] += tmp->ctr_ipm[i];
    }
    tmp=tmp->next;
  }
  
  for( i=0; i<MAXNUM_PAPI_EVENTS; i++ ) {
    reg->ctr[i] -= reg->ctr_ipm[i];
  }
#endif /* HAVE_PAPI */
  
  return ptr;
}


void* rsfunc_find_by_id(region_t *reg, unsigned level, int flags, void *ptr) 
{
  regid_t *regid;

  regid = (regid_t*)ptr;
  
  if( flags==VISIT_BACKTRACK )
    return ptr;

  if( regid->id == reg->id ) 
    regid->reg=reg;

  return ptr;
}


void* rsfunc_find_by_name(region_t *reg, unsigned level, int flags, void *ptr) 
{
  regid_t *regid;

  regid = (regid_t*)ptr;
  
  if( flags==VISIT_BACKTRACK )
    return ptr;

  if( !(regid->reg) && 
      !strcmp(regid->name, reg->name) ) {
    regid->reg=reg;
  }

  return ptr;
}



void rstack_print(region_t *rstack, FILE *f) 
{
  traverse_rstack(rstack, 0,
		  rsfunc_print_region, f);
  fprintf(stderr, "\n");

}

int rstack_count_all_regions(region_t *rstack) 
{
  int count=0;
  traverse_rstack(rstack, 0, rsfunc_enum_all_regions, &count);
  return count;
}

int rstack_count_l1_regions(region_t *rstack) 
{
  int count=0;
  traverse_rstack(rstack, 0, rsfunc_enum_l1_regions, &count);
  return count;
}


int rstack_cleanup(region_t *rstack) 
{
  traverse_rstack(rstack, 0, rsfunc_cleanup, 0);

  return IPM_OK;
}

int rstack_adjust_ctrs()
{
  //traverse_rstack(ipm_rstack, 0, rsfunc_adjust_ctrs, 0);
  
  return IPM_OK;
}

void* rsfunc_store_region(region_t *reg, unsigned level, int flags, void *ptr) 
{
  if( flags==VISIT_FIRST ) {
    /* store region with id X at index X in the array pointed to by ptr */
    /* region ids are numberered starting with 0 */
    memcpy(&(((region_t*)ptr)[reg->id]), reg, sizeof(region_t));
    /* needed for packing */
    ((region_t*)ptr)[reg->id].self=reg;
  }
  
  return ptr;
}

/* pack region stack into list, assumes list is 
   malloc'd at least sizeof(region_t)*nreg */
void rstack_pack(region_t *stack, int nreg, region_t *list) 
{
  traverse_rstack(stack, 0, rsfunc_store_region, list);
}

region_t* rstack_unpack(int nreg, region_t *list) 
{
  region_t **newregs;
  region_t *newstack;
  int i, j;

  /* unpack the regions from the list, create new region stack
     with correct pointers */

  newregs=(region_t**)IPM_MALLOC(sizeof(region_t*)*nreg);
  for( i=0; i<nreg; i++ ) {
    if( list[i].self==0 ) 
      continue;

    newregs[i] = (region_t*)IPM_MALLOC(sizeof(region_t));
    memcpy(newregs[i], &(list[i]), sizeof(region_t));
  }

  /* here we have all regions in memory again, but the pointers 
     (next, parent, child) are garbage -- fixed below*/
  
  for( i=0; i<nreg; i++ ) {
    if( list[i].self==0 ) 
      continue;

    /* child */
    for( j=0; j<nreg; j++ ) {
      if( list[j].self && newregs[i]->child==newregs[j]->self ) {
	newregs[i]->child=newregs[j];
	break;
      }
    }
    /* parent */
    for( j=0; j<nreg; j++ ) {
      if( list[j].self && newregs[i]->parent==newregs[j]->self ) {
	newregs[i]->parent=newregs[j];
	break;
      }
    }
    /* next */
    for( j=0; j<nreg; j++ ) {
      if( list[j].self && newregs[i]->next==newregs[j]->self ) {
	newregs[i]->next=newregs[j];
	break;
      }
    }
  }

  newstack = newregs[0];
  if( newregs ) IPM_FREE(newregs);

  return newstack;
}


region_t* rstack_find_region_by_id(region_t *rstack, int id) 
{
  regid_t regid;

  regid.reg = 0;
  regid.id = id;
  regid.name = 0;

  traverse_rstack(rstack, 0, rsfunc_find_by_id, &regid);
  
  return regid.reg;
}

region_t* rstack_find_region_by_name(region_t *rstack, char *name) 
{
  regid_t regid;

  regid.reg=0;
  regid.name=name;

  traverse_rstack(rstack, 0, rsfunc_find_by_name, &regid);
  
  return regid.reg;
}



#ifdef UTEST_REGSTACK

void foo() {
  ipm_region(1, "foo");
  usleep(10000);
  ipm_region(-1, "foo");
}

void bar() {
  ipm_region(1, "bar");
  foo();
  ipm_region(-1, "bar");
}

void foobar() {
  ipm_region(1, "foobar");
  foo();
  bar();
  ipm_region(-1, "foobar");
}

int main(int argc, char* argv[])
{
  int i;
  int nreg;
  region_t *rlist;
  region_t *newstack;

  rstack_init(0);

  for( i=0; i<3; i++ ) {
    foobar();
    bar();
    foobar();
  }

  /* print the original stack */
  traverse_rstack(ipm_rstack, 0,
		  rsfunc_print_region, stderr);
  fprintf(stderr, "\n");

  /* count the regions, allocate space for linearization */
  nreg = rstack_count_all_regions(ipm_rstack);
  rlist = (region_t*) malloc(sizeof(region_t)*nreg);

  fprintf(stderr, "Counted %d regions in stack\n\n", nreg);
  
  /* pack the stack in list */
  rstack_pack(ipm_rstack, nreg, rlist);

  /* clean-up old stack */
  traverse_rstack(ipm_rstack, 0, rsfunc_cleanup, 0);

  /* unpack again to get new stack */
  newstack = rstack_unpack(nreg, rlist);

  /* print new stack */
  traverse_rstack(newstack, 0,
		  rsfunc_print_region, stderr);
  fprintf(stderr, "\n");

  fprintf(stderr, "Counted %d regions in new stack\n",
	  rstack_count_all_regions(newstack));

  traverse_rstack(newstack, 0, rsfunc_cleanup, 0);  

  return 0;
}

#endif /* UTEST_REGSTACK */
