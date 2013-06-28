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
* \file process.c
 \brief Defines types and functions to deal with process of running application.
*/
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "node.h"
#include "vector.h"

/**
* Allocates memory for a process structure.
*
*/
cube_process* cube_process_create(cube_process* this) {
  if (this == NULL) {
    ALLOC(this, 1, cube_process);
  }
  if (this != NULL) cube_process_construct_child(this);
  return this;
}

/**
* Fills the structure with name and rank of process.
*
*/
void cube_process_init(cube_process* this, char* name, int rank, cube_node* parent) {
  this->name = name;
  this->rank = rank;
  this->parent = parent;
  if (parent != NULL) {
    cube_process_add_child(this->parent, this);
  }
}

/**
* Creates a child of this process.
*
*/
void cube_process_construct_child(cube_process* this) {
  XALLOC(this->child, 1, cube_tarray);
  this->child->size = 0;
  this->child->capacity = 0;
}

/**
* Ads a process "proc" to the process "this".
*
*/
void cube_process_add_child(cube_node* this, cube_process* proc) {
  ADD_NEXT(this->child, proc, cube_process *);
}

/**
* Releases memory for a process structure.
*
*/
void cube_process_free(cube_process* this) {
  if (this != NULL) {
    if (this->child != NULL) free(this->child->data);
    free(this->child);
    free(this);
  }
}

cube_thread* cube_process_get_child(cube_process* this, int i) {
  if (i < 0 || i >= this->child->size) {
    printf("cube_process_get_child: out of range\n");
  }
  return this->child->data[i];
}

cube_node* cube_process_get_parent(cube_process* this) {
  return this->parent;
}

int cube_process_get_rank(cube_process* this) {
  return this->rank;
}

char* cube_process_get_name(cube_process* this) {
  return this->name;
}

int cube_process_num_children(cube_process* this) {
  return this->child->size;
}

/**
* Gets a level of the process.
* Does it recursiv with "deep search" algorithm
*/
int cube_process_get_level(cube_process* this) {
  if (cube_process_get_parent(this) == NULL)
    return 0;
  else
    return cube_node_get_level(cube_process_get_parent(this)) + 1;
}

/**
* Writes XML output for process in to .cube file.
* Does it recursiv with "deep search" algorithm
*/
void cube_process_writeXML(cube_process* this, FILE* fp) {
  int i = 0, num = cube_process_get_level(this);
  char ind[80];

  strcpy(ind, "");
  for (i = 0; i < 2*num; i++) {
    strcat(ind, " ");
  }

  fprintf(fp, "%s    <process Id=\"%d\">\n", ind, cube_process_get_id(this));
  fprintf(fp, "%s      <name>%s</name>\n", ind, cube_process_get_name(this));
  fprintf(fp, "%s      <rank>%d</rank>\n", ind, cube_process_get_rank(this));
  for(i = 0; i < cube_process_num_children(this); i++) {
    cube_thread* thrd = cube_process_get_child(this, i);
    cube_thread_writeXML(thrd, fp);
  }
  fprintf(fp, "%s    </process>\n", ind);
}

void cube_process_set_id(cube_process* this, int new_id) {
  this->id = new_id;
}

int cube_process_get_id(cube_process* this) {
  return this->id;
}

/**
* Compares equality of two processes.
*/
int cube_process_equal(cube_process* a, cube_process* b) {
  if (cube_process_get_rank(a) == cube_process_get_rank(b))
    return 1;
  return 0;
}


