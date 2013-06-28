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
* \file metric.c
 \brief Defines types and functions to deal with metrics.
*/
#include <stdlib.h>
#include <string.h>
#include "metric.h"
#include "vector.h"

typedef struct marray {
  int size;
  int capacity;
  struct cube_metric** data;
} marray;///< Dynamical array containing metrics.

/**
* Allocates memory for the metric.
*/
cube_metric* cube_metric_create(cube_metric* this) {
  if (this == NULL) {
    ALLOC(this, 1, cube_metric);
  }
  if (this != NULL) cube_metric_construct_child(this);
  return this;
}

/**
* Fills the metric with data.
*/
void cube_metric_init(cube_metric* this, char* disp_name, char* uniq_name, char* dtype,
                      char* uom, char* val, char* url, char* descr, cube_metric* parent) {
  this->parent = parent;
  this->disp_name = disp_name;
  this->uniq_name = uniq_name;
  this->dtype = dtype;
  this->uom = uom;
  this->val = val;
  this->url = url;
  this->descr = descr;
  if (parent != NULL) {
    cube_metric_add_child(this->parent, this);
  }
}

/**
* Creates child of the metric.
*/
void cube_metric_construct_child(cube_metric* this) {
  XALLOC(this->child, 1, marray);
  this->child->size = 0;
  this->child->capacity = 0;
}

/**
* Adds child in  the metric.
*/
void cube_metric_add_child(cube_metric* parent, cube_metric* met) {
  ADD_NEXT(parent->child, met, cube_metric *);
}

/**
* Releases memory of the metric.
*/
void cube_metric_free(cube_metric* this) {
  free(this->child->data);
  free(this->child);
  if (this != NULL) free(this);
}

cube_metric* cube_metric_get_child(cube_metric* this, int i) {
  if (i < 0 || i >= this->child->size) {
    printf("cube_metric_get_child: out of range\n");
  }
  return this->child->data[i];
}

cube_metric* cube_metric_get_parent(cube_metric* this) {
  return this->parent;
}

char* cube_metric_get_disp_name(cube_metric* this) {
  return this->disp_name;
}

char* cube_metric_get_uniq_name(cube_metric* this) {
  return this->uniq_name;
}

char* cube_metric_get_dtype(cube_metric* this) {
  return this->dtype;
}

char* cube_metric_get_uom(cube_metric* this) {
  return this->uom;
}

char* cube_metric_get_val(cube_metric* this) {
  return this->val;
}

char* cube_metric_get_descr(cube_metric* this) {
  return this->descr;
}

char* cube_metric_get_url(cube_metric* this) {
  return this->url;
}

/**
* Writes a XML output belongs to the metric in to .cube file.
* Does it recursiv with "deep search" algorithm.
*/
void cube_metric_writeXML(cube_metric* this, FILE* fp) {
  int i = 0, num = cube_metric_get_level(this);
  char ind[80];

  strcpy(ind, "");
  for (i = 0; i < 2*num; i++) {
    strcat(ind, " ");
  }

  fprintf(fp, "%s    <metric id=\"%d\">\n", ind, cube_metric_get_id(this));
  fprintf(fp, "%s      <disp_name>%s</disp_name>\n", ind, cube_metric_get_disp_name(this));
  fprintf(fp, "%s      <uniq_name>%s</uniq_name>\n", ind, cube_metric_get_uniq_name(this));
  fprintf(fp, "%s      <dtype>%s</dtype>\n", ind, cube_metric_get_dtype(this));
  fprintf(fp, "%s      <uom>%s</uom>\n", ind, cube_metric_get_uom(this));
  if (strcmp(cube_metric_get_val(this), "") != 0) fprintf(fp, "%s      <val>%s</val>\n", ind, cube_metric_get_val(this));
  fprintf(fp, "%s      <url>%s</url>\n", ind, cube_metric_get_url(this));
  fprintf(fp, "%s      <descr>%s</descr>\n", ind, cube_metric_get_descr(this));

  for( i = 0; i < cube_metric_num_children(this); i++ ) {
    cube_metric* p = cube_metric_get_child(this, i);
    cube_metric_writeXML(p, fp);
  }
  fprintf(fp, "%s    </metric>\n", ind);
}

int cube_metric_num_children(cube_metric* this) {
  return this->child->size;
}

/**
* Assigns an id to this metric starting with *id.
* Does it recursiv with "deep search" algorithm.
*/
void cube_metric_assign_ids(cube_metric* this, int* id) {
  int i = 0;
  int sid = *id;
  cube_metric_set_id(this, sid);
  *id = *id + 1;
  for (i = 0; i < cube_metric_num_children(this); i++) {
    cube_metric_assign_ids(cube_metric_get_child(this, i), id);
  }
}

/**
* Gives a level of the metric. Root -> 0.
* Does it recursiv with "deep search" algorithm.
*/
int cube_metric_get_level(cube_metric* this) {
  if (cube_metric_get_parent(this) == NULL)
    return 0;
  else
    return cube_metric_get_level(cube_metric_get_parent(this)) + 1;
}

void cube_metric_set_id(cube_metric* this, int new_id) { 
  this->id = new_id; 
}

int cube_metric_get_id(cube_metric* this) { 
  return this->id; 
}

/**
* Compares equality of two metrcs.
*/
int cube_metric_equal(cube_metric* a, cube_metric* b) {
  if (strcmp(cube_metric_get_uniq_name(a), cube_metric_get_uniq_name(b)) == 0)
    return 1;
  return 0;
}
/**
* Just sets a given unique name of the metric.
*/
void cube_metric_set_uniq_name(cube_metric* this, char* uniq_name) {
  this->uniq_name = uniq_name;
}

