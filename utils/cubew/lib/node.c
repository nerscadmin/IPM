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
* \file node.c
 \brief Defines types and functions to deal with one single node of the running machine.
*/
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "process.h"
#include "machine.h"
#include "vector.h"

/**
* Allocates memory for the node.
*/
cube_node* cube_node_create(cube_node* this) {
  if (this == NULL) {
    ALLOC(this, 1, cube_node);
  }
  if (this != NULL) cube_node_construct_child(this);
  return this;
}


/**
* Fills the node with data.
*/
void cube_node_init(cube_node* this, char* name, cube_machine* parent) {
  this->name = name;
  this->parent = parent;
  if (parent != NULL) {
    cube_node_add_child(this->parent, this);
  }
}

/**
* Creates a child of the node.
*/

void cube_node_construct_child(cube_node* this) {
  XALLOC(this->child, 1, cube_parray);
  this->child->size = 0;
  this->child->capacity = 0;
}

/**
* Adds a child to the node.
*/
void cube_node_add_child(cube_machine* this, cube_node* node) {
  ADD_NEXT(this->child, node, cube_node *);
}

/**
* Releases memory for the node.
*/
void cube_node_free(cube_node* this) {
  if (this != NULL) {
    if (this->child != NULL) free(this->child->data);
    free(this->child);
    free(this);
  }
}

/**
* Returns a child of the node.
*/
cube_process* cube_node_get_child(cube_node* this, int i) {
  if (i < 0 || i >= this->child->size) {
    printf("cube_node_get_child: out of range\n");
  }
  return this->child->data[i];
}

cube_machine* cube_node_get_parent(cube_node* this) {
  return this->parent;
}

char* cube_node_get_name(cube_node* this) {
  return this->name;
}

int cube_node_num_children(cube_node* this) {
  return this->child->size;
}

int cube_node_get_level(cube_node* this) {
  if (cube_node_get_parent(this) == NULL)
    return 0;
  else
    return cube_machine_get_level(cube_node_get_parent(this)) + 1;
}

/**
* Writes XML output belongs to the node 
*/
void cube_node_writeXML(cube_node* this, FILE* fp) {
  int i = 0;
  int num = cube_node_get_level(this);
  char ind[80];

  strcpy(ind, "");
  for (i = 0; i < 2*num; i++) {
    strcat(ind, " ");
  }

  fprintf(fp, "%s    <node Id=\"%d\">\n",  ind, cube_node_get_id(this));
  fprintf(fp, "%s      <name>%s</name>\n", ind, cube_node_get_name(this));
  for(i = 0; i < cube_node_num_children(this); i++) {
    cube_process* p = cube_node_get_child(this, i);
    cube_process_writeXML(p, fp);
  }
  fprintf(fp, "%s    </node>\n", ind);
}

void cube_node_set_id(cube_node* this, int new_id) {
  this->id = new_id;
}

int cube_node_get_id(cube_node* this) {
  return this->id;
}

/**
* Compares equality of two nodes.
*/
int cube_node_equal(cube_node* a, cube_node* b) {
  if (strcmp(cube_node_get_name(a), cube_node_get_name(b)) == 0)
    return 1;
  return 0;
}


