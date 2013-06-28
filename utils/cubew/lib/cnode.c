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
* \file cnode.c
 \brief Defines types and functions to deal with calee nodes.
*/
#include <stdlib.h>
#include <string.h>

#include "cnode.h"
#include "region.h"
#include "vector.h"

typedef struct carray {
  int size;
  int capacity;
  struct cube_cnode** data;
} carray;///<  Dynamic array of cnodes.

/**
* Allocates memory for cnodes in the cube.
*/
cube_cnode* cube_cnode_create(cube_cnode* this) {
  if (this == NULL) {
    ALLOC(this, 1, cube_cnode);
  }
  if (this != NULL) cube_cnode_construct_child(this);
  return this;
}

/**
* Initialize the cnode structure with data and connect parents and children.
*/
void cube_cnode_init(cube_cnode* this, struct cube_region* callee, char* mod, int line, cube_cnode* parent) {
  this->parent = parent;
  this->callee = callee;
  this->mod = mod;
  this->line = line;
  if (parent != NULL) {
     cube_cnode_add_child(this->parent, this);
  }
}

/**
* Allocate memory for a child of some cnoed "this"
*/
void cube_cnode_construct_child(cube_cnode* this) {
  XALLOC(this->child, 1, carray);
  this->child->size = 0;
  this->child->capacity = 0;
}

/**
* Adds a child cnode "cnode" to the parent "parent"
*/
void cube_cnode_add_child(cube_cnode* parent, cube_cnode* cnode) {
  ADD_NEXT(parent->child, cnode, cube_cnode *);
}

/**
* Releases a memory of cnode "this"
*/
void cube_cnode_free(cube_cnode* this) {
  free(this->child->data);
  free(this->child);
  if (this != NULL) free(this);
}

cube_cnode* cube_cnode_get_child(cube_cnode* this, int i) {
  if (i < 0 || i >= this->child->size) {
    printf("cube_cnode_get_child: out of range\n");
  }
  return this->child->data[i];
}

cube_cnode* cube_cnode_get_parent(cube_cnode* this) {
  return this->parent;
}

int cube_cnode_get_line(cube_cnode* this) {
  return (this->line <= 0) ? -1 : this->line;
}

char* cube_cnode_get_mod(cube_cnode* this) {
  return this->mod;
}

cube_region* cube_cnode_get_callee(cube_cnode* this) {
  return this->callee;
}

cube_region* cube_cnode_get_caller(cube_cnode* this) {
  if (cube_cnode_get_parent(this) == NULL) return NULL;
  return cube_cnode_get_callee(cube_cnode_get_parent(this));
}

int cube_cnode_num_children(cube_cnode* this) {
  return this->child->size;
}

void cube_cnode_set_id(cube_cnode* this, int new_id) {
  this->id = new_id;
}

int cube_cnode_get_id(cube_cnode* this) {
  return this->id;
}
/**
* Assigns id to the cnode.
* It runs over cnodes with "deep search algorithm"
*/
void cube_cnode_assign_ids(cube_cnode* this, int* id) {
  int i = 0;
  int sid = *id;
  cube_cnode_set_id(this, sid);
  *id = *id + 1;
  for (i = 0; i < cube_cnode_num_children(this); i++) {
    cube_cnode_assign_ids(cube_cnode_get_child(this, i), id);
  }
}

int cube_cnode_get_level(cube_cnode* this) {
  if (cube_cnode_get_parent(this) == NULL)
    return 0;
  else
    return cube_cnode_get_level(cube_cnode_get_parent(this)) + 1;
}

/**
* Writes XML output for cnode in "<program>" part of the .cube file.

*/
void cube_cnode_writeXML(cube_cnode* this, FILE* fp) {
  int i, num = cube_cnode_get_level(this);

  for (i = 0; i < num; i++) fprintf(fp, "  ");
  fprintf(fp, "    <cnode id=\"%d\" ", cube_cnode_get_id(this));
  if (cube_cnode_get_line(this) != -1)
    fprintf(fp, "line=\"%d\" ", cube_cnode_get_line(this));
  if (strcmp(cube_cnode_get_mod(this), "") != 0)
    fprintf(fp, "mod=\"%s\" ", cube_cnode_get_mod(this));
  fprintf(fp, "calleeId=\"%d\">\n", cube_region_get_id(cube_cnode_get_callee(this)));
  for(i = 0; i < cube_cnode_num_children(this) ; i++) {
     cube_cnode* child = cube_cnode_get_child(this, i);
     cube_cnode_writeXML(child, fp);
  }
  for (i = 0; i < num; i++) fprintf(fp, "  ");
  fprintf(fp, "    </cnode>\n");
}

/**
* Compares on equality two cnodes.
*/
int cube_cnode_equal(cube_cnode* a, cube_cnode* b) {
  if (strcmp(cube_cnode_get_mod(a), cube_cnode_get_mod(b)) == 0) {
    if (cube_region_equal(cube_cnode_get_callee(a), cube_cnode_get_callee(b)) == 1) {
      if (cube_cnode_get_line(a) == cube_cnode_get_line(b)) {
        return 1;
      }
    }
  }
  return 0;
}

