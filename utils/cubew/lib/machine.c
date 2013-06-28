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
* \file machine.c
* \brief Defines types and functions to deal with running machine as whole object.
*/
#include <stdlib.h>
#include <string.h>

#include "machine.h"
#include "node.h"
#include "vector.h"

/**
* Allocates memory for machine.
*/
cube_machine* cube_machine_create(cube_machine* this) {
  if (this == NULL) {
    ALLOC(this, 1, cube_machine);
  }
  if (this != NULL) cube_machine_construct_child(this);
  return this;
}

/**
* Sets a general desription of the machine and its name.
*/
void cube_machine_init(cube_machine* this, char* name, char* desc) {
  this->name = name;
  this->desc = desc;
}

/**
* Allocates memory for a child machine.
*/
void cube_machine_construct_child(cube_machine* this) {
   XALLOC(this->child, 1, cube_narray);
   this->child->size = 0;
   this->child->capacity = 0;
}

/**
* Allocates memory for a nodes of the  machine.
*/
void cube_machine_reserve_nodes(cube_machine* this, unsigned num) {
  this->child->capacity = num;
  ALLOC(this->child->data, this->child->capacity, cube_node *);
}

/**
* Releases memory  of the  machine.
*/
void cube_machine_free(cube_machine* this) {
  if (this != NULL) {
    if (this->child != NULL) free(this->child->data);
    free(this->child);
    free(this);
  }
}

/**
* Returns a i-th child on the machine.
*/
cube_node* cube_machine_get_child(cube_machine* this, int i) {
  if (i < 0 || i >= this->child->size) {
    printf("cube_machine_get_child: out of range\n");
  }
  return this->child->data[i];
}

char* cube_machine_get_name(cube_machine* this) {
  return this->name;
}

char* cube_machine_get_desc(cube_machine* this) {
  return this->desc;
}

int cube_machine_num_children(cube_machine* this) {
  return this->child->size;
}

int cube_machine_get_level(cube_machine* this) {
  return 0;
}

/**
* Writes a XML output of the machine in the .cube file.
*/
void cube_machine_writeXML(cube_machine* this, FILE* fp) {
  int i = 0, num = cube_machine_get_level(this);
  char ind[80];

  strcpy(ind, "");
  for (i = 0; i < 2*num; i++) {
    strcat(ind, " ");
  }

  fprintf(fp, "%s    <machine Id=\"%d\">\n", ind, cube_machine_get_id(this));
  fprintf(fp, "%s      <name>%s</name>\n", ind, cube_machine_get_name(this));
  if (strcmp(cube_machine_get_desc(this), "") != 0) fprintf(fp, "%s      <descr>%s</descr>\n", ind, cube_machine_get_desc(this));
  for (i = 0; i < cube_machine_num_children(this); i++) {
    cube_node* node = cube_machine_get_child(this, i);
    cube_node_writeXML(node, fp);
  }
  fprintf(fp, "%s    </machine>\n", ind);
}

void cube_machine_set_id(cube_machine* this, int new_id) {
  this->id = new_id;
}

int cube_machine_get_id(cube_machine* this) {
  return this->id;
}

/**
* Compares eaquality of two machines
*/
int cube_machine_equal(cube_machine* a, cube_machine* b) {
  if (strcmp(cube_machine_get_name(a), cube_machine_get_name(b)) == 0) 
    return 1;
  return 0;
}

