/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
**  KOJAK       http://www.fz-juelich.de/jsc/kojak/                        **
*****************************************************************************
**  Copyright (c) 1998-2010                                                **
**  Forschungszentrum Juelich, Juelich Supercomputing Centre               **
**                                                                         **
**  Copyright (c) 2003-2008                                                **
**  University of Tennessee, Innovative Computing Laboratory               **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/

/**
* 
* \file cube.c
* \brief Contains a definition of functions and types related to "cube".
*
************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "cube.h"
#include "vector.h"
/**
* General flat array of pointers on general data (void*) with dynamical memory allocation
*/
typedef struct dyn_array {
  int size;
  int capacity;
  void** data;
} dyn_array;

/**
* Char mapping. Will be used for...?
*/
typedef struct cmap {
  char* key;
  char* value;
} cmap;

/**
* General flat array of pointers on 'char mapping' (cmap*)
*/

typedef struct att_array {
  int size;
  int capacity;
  cmap** data;
} att_array;

int cubew_trace = 0;


/**
* Creates and returns a data strucure cube_t*;
*/

cube_t* cube_create() {
  cube_t* this = NULL;
  cubew_trace = (getenv("CUBEW_TRACE") != NULL);
  if (cubew_trace) fprintf(stderr,"CUBEW_TRACE=%d\n", cubew_trace);
  /* allocate memory for cube */
  ALLOC(this, 1, cube_t);
  if (this == NULL) return NULL;
  /* construct dynamic arrays */
  cube_construct_arrays(this);
  this->sev_flag = 1;
  return this;
}

/**
* Creates arrays for severities. 
*/

void cube_sev_init(cube_t* this) {
  int i = 0, j = 0;

  /* Only regions defined => flat profile => create top-lecel cnodes */
  if (this->cnd_ar->size == 0 && this->reg_ar->size > 0)
    for (i = 0; i < this->reg_ar->size; i++)
      cube_def_cnode(this, (cube_region*)this->reg_ar->data[i], NULL);

  XALLOC(this->sev, this->met_ar->size, double **);
  for (i = 0; i < this->met_ar->size; i++) {
    XALLOC(this->sev[i], this->cnd_ar->size, double *);
  }

  XALLOC(this->exist, this->met_ar->size, int *);
  for (i = 0; i < this->met_ar->size; i++) {
    XALLOC(this->exist[i], this->cnd_ar->size, int);
  }
  XALLOC(this->cn_exist, this->met_ar->size, int **);
  for (i = 0; i < this->met_ar->size; i++) {
    XALLOC(this->cn_exist[i], this->cnd_ar->size, int *);
    for (j = 0; j < this->cnd_ar->size; j++) {
      XALLOC(this->cn_exist[i][j], this->thrd_ar->size, int);
    }
  } 
  cube_assign_ids(this);
}


/**
* Destroys all data in cube and removes it from memory.;
*/
void cube_free(cube_t* this) {
  int i = 0, j = 0;
  if (!this) return;
  /* free severities */
  if (this->sev) {
    for (i = 0; i < this->met_ar->size; i++) {
      for (j = 0; j < this->cnd_ar->size; j++) {
        free(this->sev[i][j]);
      }
      free(this->sev[i]);
    }
    free(this->sev);
  }
  if (this->exist) {
    for (i = 0; i < this->met_ar->size; i++) {
      free(this->exist[i]);
    }
    free(this->exist);
  }
  if (this->cn_exist) {
    for (i = 0; i < this->met_ar->size; i++) {
      for (j = 0; j < this->cnd_ar->size; j++)
        free(this->cn_exist[i][j]);
      free(this->cn_exist[i]);
    }
    free(this->cn_exist);
  }
  /* free metrics */
  if (this->met_ar) {
    for (i = 0; i < this->met_ar->size; i++) {
      cube_metric_free(this->met_ar->data[i]); 
    }
    free(this->met_ar->data);
    free(this->met_ar);
  }
  if (this->rmet_ar) {
    free(this->rmet_ar->data);
    free(this->rmet_ar);
  }
  /* free regions */
  if (this->reg_ar) {
    for (i = 0; i < this->reg_ar->size; i++) {
      cube_region_free(this->reg_ar->data[i]);
    }
    free(this->reg_ar->data);
    free(this->reg_ar);
  }
  /* free cnodes */
  if (this->cnd_ar) {
    for (i = 0; i < this->cnd_ar->size; i++) {
      cube_cnode_free(this->cnd_ar->data[i]);
    }
    free(this->cnd_ar->data);
    free(this->cnd_ar);
  }
  if (this->rcnd_ar) {
    free(this->rcnd_ar->data);
    free(this->rcnd_ar);
  }
  /* free machines */
  if (this->mach_ar) {
    for (i = 0; i < this->mach_ar->size; i++) {
      cube_machine_free(this->mach_ar->data[i]);
    }
    free(this->mach_ar->data);
    free(this->mach_ar);
  }
  /* free nodes */
  if (this->node_ar) {
    for (i = 0; i < this->node_ar->size; i++) {
      cube_node_free(this->node_ar->data[i]);
    }
    free(this->node_ar->data);
    free(this->node_ar);
  }
  /* free processes */
  if (this->proc_ar) {
    for (i = 0; i < this->proc_ar->size; i++) {
      cube_process_free(this->proc_ar->data[i]);
    }
    free(this->proc_ar->data);
    free(this->proc_ar);
  }
  /* free threads */
  if (this->thrd_ar) {
    for (i = 0; i < this->thrd_ar->size; i++) {
      cube_thread_free(this->thrd_ar->data[i]);
    }
    free(this->thrd_ar->data);
    free(this->thrd_ar);
  }
  /* free attrs */
  if (this->attr) {
    for (i = 0; i < this->attr->size; i++) {
      free(this->attr->data[i]); 
    }
    free(this->attr->data);
    free(this->attr);
  }
  /* free mirrors */
  if (this->mirr_ar) {
    if (this->mirr_ar->data) free(this->mirr_ar->data);
    free(this->mirr_ar);
  }
  /* free cartesians */
  for (i = 0; i < this->cart_ar->size; i++) {
    free(this->cart_ar->data[i]); 
  }
  if (this->cart_ar) {
    if (this->cart_ar->data) free(this->cart_ar->data);
    free(this->cart_ar);
  }
  /* lastly, delete the cube object */
  free(this); 
}


/**
* Creates  empty dimensional arrays in the cube.
*/
void cube_construct_arrays(cube_t* this) {
  /* construct metric array */
  XALLOC(this->met_ar, 1, dyn_array);
  this->met_ar->size = 0; 
  this->met_ar->capacity = 0;

  /* construct root metric array */
  XALLOC(this->rmet_ar, 1, dyn_array);
  this->rmet_ar->size = 0; 
  this->rmet_ar->capacity = 0;

  /* construct region array */
  XALLOC(this->reg_ar, 1, dyn_array);
  this->reg_ar->size = 0; 
  this->reg_ar->capacity = 0;

  /* construct cnode array */
  XALLOC(this->cnd_ar, 1, dyn_array);
  this->cnd_ar->size = 0; 
  this->cnd_ar->capacity = 0;

  /* construct root cnode array */
  XALLOC(this->rcnd_ar, 1, dyn_array);
  this->rcnd_ar->size = 0; 
  this->rcnd_ar->capacity = 0;

  /* construct machine array */
  XALLOC(this->mach_ar, 1, dyn_array);
  this->mach_ar->size = 0; 
  this->mach_ar->capacity = 0;

  /* construct node array */
  XALLOC(this->node_ar, 1, dyn_array);
  this->node_ar->size = 0; 
  this->node_ar->capacity = 0;

  /* construct process array */
  XALLOC(this->proc_ar, 1, dyn_array);
  this->proc_ar->size = 0; 
  this->proc_ar->capacity = 0;

  /* construct thread array */
  XALLOC(this->thrd_ar, 1, dyn_array);
  this->thrd_ar->size = 0; 
  this->thrd_ar->capacity = 0;

  /* construct cartesian array */
  XALLOC(this->cart_ar, 1, dyn_array);
  this->cart_ar->size = 0; 
  this->cart_ar->capacity = 0;

  /* construct mirror_url array */
  XALLOC(this->mirr_ar, 1, dyn_array);
  this->mirr_ar->size = 0; 
  this->mirr_ar->capacity = 0;

  /* construct attrs array */
  XALLOC(this->attr, 1, att_array);
  this->attr->size = 0; 
  this->attr->capacity = 0;
}

/** 
* Allocates memory for "num" metrics (and rmetrics) in the cube "this"
* Returns 0 - succes, non 0 - failed (not enough memory or whatever)
*/
int cube_reserve_metrics(cube_t* this, unsigned num) {
  this->met_ar->capacity = num;
  ALLOC(this->met_ar->data, this->met_ar->capacity, void *);
  this->rmet_ar->capacity = num;
  ALLOC(this->rmet_ar->data, this->rmet_ar->capacity, void *);
  return (this->rmet_ar->data == NULL);
}

/** 
* Allocates memory for "num" regions in the cube "this"
* Returns 0 - succes, non 0 - failed (not enough memory or whatever)
*/
int cube_reserve_regions(cube_t* this, unsigned num) {
  this->reg_ar->capacity = num;
  ALLOC(this->reg_ar->data, this->reg_ar->capacity, void *);
  return (this->reg_ar->data == NULL);
}

/** 
* Allocates memory for "num" calees in the cube "this"
* Returns 0 - succes, non 0 - failed (not enough memory or whatever)
*/
int cube_reserve_cnodes(cube_t* this, unsigned num) {
  this->cnd_ar->capacity = num;
  ALLOC(this->cnd_ar->data, this->cnd_ar->capacity, void *);
  return (this->cnd_ar->data == NULL);
}

/** 
* Allocates memory for "num" calees in the cube "this"
* Returns 0 - succes, non 0 - failed (not enough memory or whatever)
*/
int cube_reserve_machs(cube_t* this, unsigned num) {
  this->mach_ar->capacity = num;
  ALLOC(this->mach_ar->data, this->mach_ar->capacity, void *);
  return (this->mach_ar->data == NULL);
}

/** 
* Allocates memory for "num" nodes of a machine in the cube "this"
* Returns 0 - succes, non 0 - failed (not enough memory or whatever)
*/
int cube_reserve_nodes(cube_t* this, unsigned num) {
  this->node_ar->capacity = num;
  ALLOC(this->node_ar->data, this->node_ar->capacity, void *);
  /* also reserve space in machine ? */
  if (this->mach_ar->capacity == 1) {
    cube_machine_reserve_nodes(this->mach_ar->data[0], num);
  }
  return (this->node_ar->data == NULL);
}

/** 
* Allocates memory for "num" processes  in the cube "this"
* Returns 0 - succes, non 0 - failed (not enough memory or whatever)
*/
int cube_reserve_procs(cube_t* this, unsigned num) {
  this->proc_ar->capacity = num;
  ALLOC(this->proc_ar->data, this->proc_ar->capacity, void *);
  return (this->proc_ar->data == NULL);
}

/** 
* Allocates memory for "num" threads  in the cube "this"
* Returns 0 - succes, non 0 - failed (not enough memory or whatever)
*/
int cube_reserve_thrds(cube_t* this, unsigned num) {
  this->thrd_ar->capacity = num;
  ALLOC(this->thrd_ar->data, this->thrd_ar->capacity, void *);
  return (this->thrd_ar->data == NULL);
}

/** 
* Allocates memory for "num" elements of cartesian topology  in the cube "this"
* Returns 0 - succes, non 0 - failed (not enough memory or whatever)
*/
int cube_reserve_carts(cube_t* this, unsigned num) {
  this->cart_ar->capacity = num;
  ALLOC(this->cart_ar->data, this->cart_ar->capacity, void *);
  return (this->cart_ar->data == NULL);
}

/** 
* Adds a metric "met" into cube "this"
*/
void cube_add_metric(cube_t* this, cube_metric* met) {
  ADD_NEXT(this->met_ar, met, void*);
}

/** 
* Adds a rmetric "met" into cube "this"
*/
void cube_add_rmetric(cube_t* this, cube_metric* met) {
  ADD_NEXT(this->rmet_ar, met, void*);
}

/** 
* Adds a calee node "cnd" into cube "this"
*/
void cube_add_cnode(cube_t* this, cube_cnode* cnd) {
  ADD_NEXT(this->cnd_ar, cnd, void*);
}

/** 
* Adds a calee rnode "cnd" into cube "this"
*/
void cube_add_rcnode(cube_t* this, cube_cnode* cnd) {
  ADD_NEXT(this->rcnd_ar, cnd, void*);
}

/** 
* Adds a region of source code "reg" into cube "this"
*/
void cube_add_region(cube_t* this, cube_region* reg) {
  ADD_NEXT(this->reg_ar, reg, void*);
}

/** 
* Adds a machine "mach" into cube "this"
*/
void cube_add_mach(cube_t* this, cube_machine* mach) {
  cube_machine_set_id(mach, this->mach_ar->size);
  ADD_NEXT(this->mach_ar, mach, void*);
}

/** 
* Adds a node "node" into cube "this"
*/
void cube_add_node(cube_t* this, cube_node* node) {
  cube_node_set_id(node, this->node_ar->size);
  ADD_NEXT(this->node_ar, node, void*);
}

/** 
* Adds a process "proc" into cube "this"
*/
void cube_add_proc(cube_t* this, cube_process* proc) {
  cube_process_set_id(proc, this->proc_ar->size);
  ADD_NEXT(this->proc_ar, proc, void*);
}

/** 
* Adds a thread "thrd" into cube "this"
*/
void cube_add_thrd(cube_t* this, cube_thread* thrd) {
  cube_thread_set_id(thrd, this->thrd_ar->size);
  ADD_NEXT(this->thrd_ar, thrd, void*);
}

/** 
* Adds a cartesian topology "cart" into cube "this"
*/
void cube_add_cart(cube_t* this, cube_cartesian* cart) {
  ADD_NEXT(this->cart_ar, cart, void*);
}

/** 
* Adds a mirror  "mir" into cube "this"
*/
void cube_add_mirror(cube_t* this, const char* mir) {
  char* nmir = strdup(mir);
  ADD_NEXT(this->mirr_ar, nmir, void*);
}

/** 
* Adds an attribute  "m" into cube "this"
*/
void cube_add_attr(cube_t* this, cmap* m) {
  ADD_NEXT(this->attr, m, cmap*);
}


/** 
* Creates and add in to cube "this" the attribute "key" with a vaue "value"
*/
void cube_def_attr(cube_t* this, const char* key, const char* value) {
  char* nkey = strdup(key);
  char* nvalue = strdup(value);
  cmap* XALLOC(m, 1, cmap);
  m->key = nkey;
  m->value = nvalue;
  cube_add_attr(this, m); 
}


/** 
* Creates and add in to cube "this" the mirror with an  url "url"
*/
void cube_def_mirror(cube_t* this, const char* url) {
  char* nurl = strdup(url);
  cube_add_mirror(this, nurl);
}


/** 
* Creates and add in to cube "this" the metric.
*/
cube_metric* cube_def_met(cube_t* this, const char* disp_name, const char* uniq_name, const char* dtype, const char* uom, 
     		          const char* val, const char* url, const char* descr, cube_metric* parent) { 
  char* ndisp = strdup(disp_name);
  char* nuniq = strdup(uniq_name);
  char* ndtype = strdup(dtype);
  char* nuom = strdup(uom);
  char* nval = strdup(val);
  char* nurl = strdup(url);
  char* ndescr = strdup(descr);
  cube_metric* met = cube_metric_create(NULL); 
  cube_metric_init(met, ndisp, nuniq, ndtype, nuom, nval, nurl, ndescr, parent);
  if (parent == NULL) {
    cube_add_rmetric(this, met);
  }
  cube_add_metric(this, met);
  return met;
}


/** 
* Creates and add in to cube "this" the region.
*/
cube_region* cube_def_region(cube_t* this, const char* name, long begln, long endln, 
		        const char* url, const char* descr, const char* mod) {
  char* nname = strdup(name);
  char* nurl = strdup(url);
  char* ndescr = strdup(descr);
  char* nmod = strdup(mod);
  cube_region* reg = cube_region_create(NULL); 
  cube_region_init(reg, nname, begln, endln, nurl, ndescr, nmod);
  cube_add_region(this, reg);
  return reg;
}


/** 
* Creates and add in to cube "this" the calee cnode with available source code information.
*/
cube_cnode* cube_def_cnode_cs(cube_t* this, cube_region *callee, const char* mod, int line, cube_cnode* parent) {
  char* nmod = strdup(mod);
  cube_cnode* cnode = cube_cnode_create(NULL); 
  if (!cnode) return NULL;
  cube_cnode_init(cnode, callee, nmod, line, parent);
  if (parent == NULL) {
    cube_add_rcnode(this, cnode);
  }
  else {
    cube_region* caller = cube_cnode_get_callee(parent);
    cube_region_add_cnode(caller, cnode);
  }
  cube_add_cnode(this, cnode);
  return cnode;
}


/** 
* Creates and add in to cube "this" the region without available source code information.
*/
cube_cnode* cube_def_cnode(cube_t* this, cube_region* callee, cube_cnode* parent) {
  /** if source code info not available, use empty string for mod */
  /** and -1 for line # */
  cube_cnode* cnode = cube_cnode_create(NULL); 
  cube_cnode_init(cnode, callee, "", 0, parent);
  if (parent == NULL) {
    cube_add_rcnode(this, cnode);
  }
  else { 
    cube_region* caller = cube_cnode_get_callee(parent);
    cube_region_add_cnode(caller, cnode);
  }
  cube_add_cnode(this, cnode);
  return cnode;
}


/** 
* Creates and add in to cube "this" the machine.
*/
cube_machine* cube_def_mach(cube_t* this, const char* name, const char* desc) {
  char* nname = strdup(name);
  char* ndesc = strdup(desc);
  cube_machine* mach = cube_machine_create(NULL); 
  cube_machine_init(mach, nname, ndesc);
  cube_add_mach(this, mach);
  return mach;
}


/** 
* Creates and add in to cube "this" the node of a machine.
*/
cube_node* cube_def_node(cube_t* this, const char* name, cube_machine* mach) {
  char* nname = strdup(name);
  cube_node* node = cube_node_create(NULL); 
  cube_node_init(node, nname, mach);
  cube_add_node(this, node);
  return node;
}

/** 
* Creates and add in to cube "this" the process.
*/
cube_process* cube_def_proc(cube_t* this, const char* name, int rank, cube_node* node) {
  char* nname = strdup(name);
  cube_process* proc = cube_process_create(NULL); 
  cube_process_init(proc, nname, rank, node);
  cube_add_proc(this, proc);
  return proc;
}


/** 
* Creates and add in to cube "this" the thread.
*/
cube_thread* cube_def_thrd(cube_t* this, const char* name, int rank, cube_process* proc) {
  char* nname = strdup(name);
  cube_thread* thrd = cube_thread_create(NULL); 
  cube_thread_init(thrd, nname, rank, proc);
  cube_add_thrd(this, thrd);
  return thrd;
}

/** 
* Creates and add in to cube "this" the cartesian topology.
* It does support only dim<=3 for cartesian topologies.
*/
cube_cartesian* cube_def_cart(cube_t* this, long int ndims, long int* dim, int* period) {
  cube_cartesian* newc;
  if (ndims > 3) {
    fprintf(stderr,"cubew: WARNING: CUBE3 doesn't support Cartesian topologies with %ld dimensions\n", ndims);
    return NULL;
  }
  newc = cube_cart_create(NULL);
  if (cube_cart_init(newc, ndims, dim, period) != 0) {
    fprintf(stderr,"cubew: WARNING: failed to create definition for Cartesian topology\n");
    cube_cart_free(newc);
    return NULL;
  }
  cube_add_cart(this, newc);
  return newc;
}



void cube_def_coords(cube_t* this, cube_cartesian* cart, cube_thread* thrd, long int* coord) {
  if (!cart)
    return;
  cube_cart_def_coords(cart, thrd, coord);
}



/**
* Set a quartet (metric, cnode, thread, value) in a cube. 
*/
void cube_set_sev(cube_t* this, cube_metric* met, cube_cnode* cnode, cube_thread* thrd, double value) {
  if (this->sev_flag == 1) {
    cube_sev_init(this);
    this->sev_flag = 0;
  }
#if DEBUG
  printf("metric_id=%d cnode_id=%d thread_id=%d exist=%d cn_exist=%d ",
        cube_metric_get_id(met), cube_cnode_get_id(cnode), cube_thread_get_id(thrd),
        this->exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)],
        this->cn_exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)]);
  if (this->sev[cube_metric_get_id(met)][cube_cnode_get_id(cnode)])
      printf("value=%g", this->sev[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)]);
  else printf("NOVALUE");
  printf(" -> %g\n", value);
#endif
 
  /* if the pointer doesn't exist, create it and store the value */
  if (this->exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)] == 0) {
    XALLOC(this->sev[cube_metric_get_id(met)][cube_cnode_get_id(cnode)], this->thrd_ar->size, double);
    this->exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)] = 1;
    this->sev[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)] = value;
    this->cn_exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)] = 1;
  }
  /* if it already contains a value, add the new value to it */
  else if (this->exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)] == 1 && 
           this->cn_exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)] == 1) {
    this->sev[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)] += value;
  }
  /* if the pointer exists, but contains no value, store the value */
  else {
    this->sev[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)] = value;
    this->cn_exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)] = 1;
  }
}


/**
* Set a quartet (metric, cnode, thread, value) in a cube for specific "region".
* The algorithm looks first for calee for given region and then sets a quartet. 
*/
void cube_set_sev_reg(cube_t* this, cube_metric* met, cube_region* region, cube_thread* thrd, double value) {
  int i = 0;
  cube_cnode* v_cnode = NULL;
  if (this->sev_flag == 1) {
    cube_sev_init(this);
    this->sev_flag = 0;
  }
  for (i = 0; i < this->cnd_ar->size; i++) {
    cube_region* tmp = cube_cnode_get_callee(this->cnd_ar->data[i]);
    if (cube_region_equal(tmp, region) == 1) {
      v_cnode = this->cnd_ar->data[i];
      break;
    }
  }

  /* This should not happen. If it does, someone is providing an undefined
     region or trying to create a mixture of flat/cnode profiles. */
  if (v_cnode == NULL) {
    fprintf(stderr, "cube_set_sev_reg: Region undefined or trying to create a mixed flat/calltree profile\n");
    exit(3);
  }

  cube_set_sev(this, met, v_cnode, thrd, value);
}

/**
* Add to the value of  quartet (metric, cnode, thread, value) in a cube a "incr" value.
* The algorithm looks first for old value "val" and then set as new one "val+incr". 
*/
void cube_add_sev(cube_t* this, cube_metric* met, cube_cnode* cnode, cube_thread* thrd, double incr) {
  double val;
  if (this->sev_flag == 1) {
    cube_sev_init(this);
    this->sev_flag = 0;
  }
  val = cube_get_sev(this, met, cnode, thrd);
  cube_set_sev(this, met, cnode, thrd, val+incr);
}

/**
* Add to the value of  quartet (metric, cnode, thread, value) in a cube a "incr" value for specific "region".
* The algorithm looks first for old value "val" and then set as new one "val+incr". 
*/
void cube_add_sev_reg(cube_t* this, cube_metric* met, cube_region* region, cube_thread* thrd, double incr) {
  cube_cnode* cn = NULL;
  int i = 0;
  double val;
  if (this->sev_flag == 1) {
    cube_sev_init(this);
    this->sev_flag = 0;
  }
  for (i = 0; i < this->cnd_ar->size; i++) {
    cube_region* tmp = cube_cnode_get_callee(this->cnd_ar->data[i]);
    if (cube_region_equal(tmp, region) == 1) {
      cn = this->cnd_ar->data[i];
      break;
    }
  }

  /* This should not happen. If it does, someone is providing an undefined
     region or trying to create a mixture of flat/cnode profiles. */
  if (cn == NULL) {
    fprintf(stderr, "cube_add_sev_reg: Region undefined or trying to create a mixed flat/calltree profile\n");
    exit(3);
  }

  val = cube_get_sev(this, met, cn, thrd);
  cube_set_sev(this, met, cn, thrd, val+incr);
}
/**
* Assgns 'id' to every element of dimensions.
* Starts with 0 and proceed recursiv.
*
* ids for machines/nodes/processes/threads assigned when created 
*/

void cube_assign_ids(cube_t* this) {
  int id = 0;
  int i = 0;

  /* metrics */
  for (i = 0; i < this->rmet_ar->size; i++) {
    cube_metric_assign_ids(this->rmet_ar->data[i], &id);
  }
      
  /* regions */
  for (i = 0; i < this->reg_ar->size; i++)
    cube_region_set_id(this->reg_ar->data[i], i);
      
  /* cnodes */
  id = 0;
  for (i = 0; i < this->rcnd_ar->size; i++) {
    cube_cnode_assign_ids(this->rcnd_ar->data[i], &id);
  }

  /* ids for machines/nodes/processes/threads assigned when created */
}
  
dyn_array* cube_get_rmet(cube_t* this) {
  return this->rmet_ar;
}

dyn_array* cube_get_met(cube_t* this) {
  return this->met_ar;
}

dyn_array* cube_get_rcnd(cube_t* this) {
  return this->rcnd_ar;
}

dyn_array* cube_get_cnd(cube_t* this) {
  return this->cnd_ar;
}

dyn_array* cube_get_mirr(cube_t* this) {
  return this->mirr_ar;
}

dyn_array* cube_get_reg(cube_t* this) {
  return this->reg_ar;
}

dyn_array* cube_get_mach(cube_t* this) {
  return this->mach_ar;
}

dyn_array* cube_get_thrd(cube_t* this) {
  return this->thrd_ar;
}

dyn_array* cube_get_cart(cube_t* this) {
  return this->cart_ar;
}

att_array* cube_get_attr(cube_t* this) {
  return this->attr;
}

/**
* Returns a value of severity for given (metic, cnode, thread)
*/

double cube_get_sev(cube_t* this, cube_metric* met, cube_cnode*  cnode, cube_thread* thrd) {
  if (this->exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)] == 1) {
    if (this->cn_exist[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)] == 1) {
      return this->sev[cube_metric_get_id(met)][cube_cnode_get_id(cnode)][cube_thread_get_id(thrd)];
    }
    else return 0; 
  }
  return 0;
}

/**
* Writes a .cube file. 
* First defitions, then matrix with values.
*/
void cube_write_all(cube_t* this, FILE* fp) {
  cube_write_def(this, fp);
  cube_write_sev_matrix(this, fp);
}

/**
* Writes a definitions-part of a .cube file. 
*/
void cube_write_def(cube_t* this, FILE* fp) {
  dyn_array* rmet = cube_get_rmet(this);
  dyn_array* rcnd = cube_get_rcnd(this);
  dyn_array* reg  = cube_get_reg(this);
  dyn_array* mach = cube_get_mach(this);
  dyn_array* cart = cube_get_cart(this);
  dyn_array* mirr = cube_get_mirr(this);
  att_array* attr = cube_get_attr(this);

  int i = 0;

  cube_assign_ids(this);

  /* xml header */
  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n");
  fprintf(fp, "<cube version=\"3.0\">\n");

  /* metadata info */
  for (i = 0; i < attr->size; i++) {
    char* key = attr->data[i]->key;
    char* value = attr->data[i]->value;
    fprintf(fp, "  <attr key=\"%s\" value=\"%s\"/>\n", key, value);
  } 

  /* mirrored URLs */
  fprintf(fp, "  <doc>\n");
  fprintf(fp, "    <mirrors>\n");
  for (i = 0; i < mirr->size; i++) {
    char* mirror = (char *) mirr->data[i];
    fprintf(fp, "      <murl>%s</murl>\n", mirror);
  }
  fprintf(fp, "    </mirrors>\n"); 
  fprintf(fp, "  </doc>\n");

  /* metrics */
  fprintf(fp, "  <metrics>\n");
  for (i = 0; i < rmet->size; i++) {  /* write metrics */
    cube_metric* m = (cube_metric *) rmet->data[i];
    cube_metric_writeXML(m, fp);  
  }
  fprintf(fp, "  </metrics>\n");

  /* program */
  fprintf(fp, "  <program>\n");

  for (i = 0; i < reg->size; i++) {    /* write regions */
    cube_region* r = (cube_region *) reg->data[i];
    cube_region_writeXML(r, fp);
  }

  for (i = 0; i < rcnd->size; i++) {   /* write cnodes */
    cube_cnode* c = (cube_cnode*) rcnd->data[i];
    cube_cnode_writeXML(c, fp);
  }

  fprintf(fp, "  </program>\n");

  /* system */
  fprintf(fp, "  <system>\n");
  for (i = 0; i < mach->size; i++) {    /* write system resources */
    cube_machine* m = (cube_machine *) mach->data[i];
    cube_machine_writeXML(m, fp);
  }

  /* topologies */
  fprintf(fp, "    <topologies>\n");
  for (i = 0; i < cart->size; i++) { /* write topologies */
    cube_cartesian *c = (cube_cartesian *) cart->data[i];
    cube_cart_writeXML(c, fp);      
  }
  fprintf(fp, "    </topologies>\n");
  fprintf(fp, "  </system>\n"); 
}

/**
* Writes recursiv a values-part of a .cube file. 
*/
void cube_write_sev_matrix(cube_t* this, FILE* fp) {
  int i = 0, j = 0, k = 0, p = 0;
  int all_zero = 1;

  dyn_array* met  = cube_get_met(this);
  dyn_array* cnd  = cube_get_cnd(this);
  dyn_array* thrd = cube_get_thrd(this);

  /* severity */
  fprintf(fp, "  <severity>\n");

  for (i = 0; i < met->size; i++) {
    cube_metric* m = (cube_metric *) met->data[i];
    fprintf(fp, "    <matrix metricId=\"%d\">\n", cube_metric_get_id(m));
    for (j = 0; j < cnd->size; j++) {
      cube_cnode* c = (cube_cnode *) cnd->data[j];
      int ztest = 0;
      for (p = 0; p < thrd->size; p++) {
        cube_thread* t = (cube_thread *) thrd->data[p];
        if (cube_get_sev(this, m, c, t) != 0.0) { all_zero = 0; break; }
        else all_zero = 1;
      }
      if (all_zero == 0) {
        for (k = 0; k < thrd->size; k++) {
          cube_thread* t = (cube_thread *) thrd->data[k];
          /* if (cube_get_sev(this, m, c, t) != 0)  { */
            if (ztest == 0) {
              ztest = 1;
              fprintf(fp, "      <row cnodeId=\"%d\">\n", cube_cnode_get_id(c));
            }
            /* fprintf(fp, "        <val thrdId=\"%d\">", cube_thread_get_id(t)); */
            /* fprintf(fp, "%.12g</val>\n", cube_get_sev(this, m, c, t)); */
            fprintf(fp, "%.12g\n", cube_get_sev(this, m, c, t));
          /* } */
        }
        if (ztest)
          fprintf(fp, "      </row>\n"); 
      }
    }
    fprintf(fp, "    </matrix>\n"); 
  }

  fprintf(fp, "  </severity>\n"); 
  fprintf(fp, "</cube>\n");
  this->sev_flag = -1;
}


/**
* writes the "sevs" as a row over "threads" for given combination "metric, caleenode".
*/
void cube_write_sev_row(cube_t* this, FILE* fp, cube_metric* met, cube_cnode* cnd, double* sevs) {
  static int first = 1;
  static int curr = -1;
  int i = 0, p = 0;
  int all_zero = 1;

  if (first) {
    fprintf(fp, "  <severity>\n");
    this->sev_flag = 0;
    first = 0;
  }

  if (cube_metric_get_id(met) != curr) {
    if (curr != -1) fprintf(fp, "    </matrix>\n");
    if (strcmp(cube_metric_get_val(met), "VOID") == 0) return;
    fprintf(fp, "    <matrix metricId=\"%d\">\n", cube_metric_get_id(met));
    curr = cube_metric_get_id(met);
  }

  /** XXXX WARNING XXXX
     The following code uses each thread's parent process rank to index into
     the vector of severity values (sevs).  By tracking the previously used
     process rank (last_prank), it avoids replicating the values for threads
     that have the same process (at least when reported in process order).
     Use cube_write_sev_threads when real thread severities are provided.
  */

  all_zero = 1;
  for (p = 0; p < this->proc_ar->size; p++) {
    if (sevs[p] != 0.0) {
      all_zero = 0;
      break;
    }
  }

  if (all_zero == 0 && this->thrd_ar->size > 0) {
    int last_prank = -1;

    /** Write severity data */
    fprintf(fp, "      <row cnodeId=\"%d\">\n", cube_cnode_get_id(cnd));
    for (i = 0; i < this->thrd_ar->size; i++) {
        int prank = cube_process_get_rank(cube_thread_get_parent(this->thrd_ar->data[i]));
        if (prank == last_prank)
          fprintf(fp, "0\n");
        else
          fprintf(fp, "%.12g\n", sevs[prank]);
        last_prank = prank;
    }
    fprintf(fp, "      </row>\n"); 
  }
}

/** severities are those for teams of threads on each process rank in sequence */
void cube_write_sev_threads(cube_t* this, FILE* fp, cube_metric* met, cube_cnode* cnd, double* sevs) {
  static int first = 1;
  static int curr = -1;
  int i = 0, p = 0;
  int all_zero = 1;

  if (first) {
    fprintf(fp, "  <severity>\n");
    this->sev_flag = 0;
    first = 0;
  }

  if (cube_metric_get_id(met) != curr) {
    if (curr != -1) fprintf(fp, "    </matrix>\n");
    if (strcmp(cube_metric_get_val(met), "VOID") == 0) return;
    fprintf(fp, "    <matrix metricId=\"%d\">\n", cube_metric_get_id(met));
    curr = cube_metric_get_id(met);
  }

  all_zero = 1;
  for (p = 0; p < this->thrd_ar->size; p++) {
    if (sevs[p] != 0.0) {
      all_zero = 0;
      break;
    }
  }

  if (!all_zero && this->thrd_ar->size > 0) {
    /* Write severity data */
    fprintf(fp, "      <row cnodeId=\"%d\">\n", cube_cnode_get_id(cnd));
    for (i = 0; i < this->thrd_ar->size; i++) {
        fprintf(fp, "%.12g\n", sevs[i]);
    }
    fprintf(fp, "      </row>\n"); 
  }
}

/**
* writes end of the .cube file. used in couple with "void cube_write_sev_row"
*/
void cube_write_finish(cube_t* this, FILE* fp) {
  if (this->sev_flag == 0) {
    fprintf(fp, "    </matrix>\n"); 
    fprintf(fp, "  </severity>\n"); 
  }
  fprintf(fp, "</cube>\n"); 
}
