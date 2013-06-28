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
* \file thread.c
 \brief Defines types and functions to deal with threads of running application.
*/
#include <stdlib.h>
#include <string.h>
#include "thread.h"
#include "process.h"
#include "vector.h"

/**
* Allocates memory for the structure "thread"
*/
cube_thread* cube_thread_create(cube_thread* this) {
  if (this == NULL) {
    ALLOC(this, 1, cube_thread);
  }
  if (this != NULL) cube_thread_construct_child(this);
  return this;
}


/**
* Fills the thread with the information (rank, namem, id).
*
*/
void cube_thread_init(cube_thread* this, char* name, int rank, cube_process* parent) {
  this->name = name;
  this->rank = rank;
  this->parent = parent;
  if (parent != NULL) {
    cube_thread_add_child(this->parent, this);
  }
}

/**
* Constructs a child of this thread.
*
*/
void cube_thread_construct_child(cube_thread* this) {
  XALLOC(this->child, 1, cube_tarray);
  this->child->size = 0;
  this->child->capacity = 0;
}

/**
* Adds a child of this thread.
*
*/
void cube_thread_add_child(cube_process* this, cube_thread* thrd) {
  ADD_NEXT(this->child, thrd, cube_thread *);
}

/**
* Releases memory of the structure "thread"
*/
void cube_thread_free(cube_thread* this) {
  if (this != NULL) {
    if (this->child != NULL) free(this->child->data);
    free(this->child);
    free(this);
  }
}

cube_thread* cube_thread_get_child(cube_thread* this, int i) {
  if (i < 0 || i >= this->child->size) {
    printf("cube_thread_get_child: out of range\n");
  }
  return this->child->data[i];
}

cube_process* cube_thread_get_parent(cube_thread* this) {
  return this->parent;
}

char* cube_thread_get_name(cube_thread* this) {
  return this->name;
}

int cube_thread_get_rank(cube_thread* this) {
  return this->rank;
}

int cube_thread_num_children(cube_thread* this) {
  return this->child->size;
}

int cube_thread_get_level(cube_thread* this) {
  if (cube_thread_get_parent(this) == NULL)
    return 0;
  else
    return cube_process_get_level(cube_thread_get_parent(this)) + 1;
}

/**
* Writes XML output for thread in to .cube file.
* No recursiv. Plain one after another. 
*
*/
void cube_thread_writeXML(cube_thread* this, FILE* fp) {
  char ind[80];
  int i = 0, num = cube_thread_get_level(this);

  strcpy(ind, "");
  for (i = 0; i < 2*num; i++) {
    strcat(ind, " ");
   }

  fprintf(fp, "%s    <thread Id=\"%d\">\n", ind, cube_thread_get_id(this));
  fprintf(fp, "%s      <name>%s</name>\n", ind, cube_thread_get_name(this));
  fprintf(fp, "%s      <rank>%d</rank>\n", ind, cube_thread_get_rank(this));
  fprintf(fp, "%s    </thread>\n", ind);
}

void cube_thread_set_id(cube_thread* this, int new_id) {
  this->id = new_id;
}

int cube_thread_get_id(cube_thread* this) {
  return this->id;
}

/**
* Compares equality of two threads.
*/
int cube_thread_equal(cube_thread* a, cube_thread* b) {
  if (cube_thread_get_rank(a) == cube_thread_get_rank(b)) {
    if (cube_process_get_rank(cube_thread_get_parent(a)) == cube_process_get_rank(cube_thread_get_parent(b))) 
      return 1;
  }
  return 0;
}


