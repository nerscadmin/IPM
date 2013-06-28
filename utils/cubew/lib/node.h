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
* \file node.h
 \brief Declares types and functions to deal with one single node of the running machine.
*/
#ifndef CUBEW_NODE_H
#define CUBEW_NODE_H   

#ifdef __cplusplus
extern "C" {
#endif

  #include <stdio.h>
  #include "process.h"

  struct cube_machine;
/**
A dynamical array of nodes.
*/
  typedef struct cube_narray {
    int size;
    int capacity;
    struct cube_node** data;
  } cube_narray;

/**
A description and ID of a node. And its children.
*/
  typedef struct cube_node {
    cube_parray* child;
    struct cube_machine* parent;
    char* name;
    int id;
  } cube_node;

  cube_node* cube_node_create(cube_node* node);
  void   cube_node_init(cube_node* node, char* name, struct cube_machine* parent);
  void   cube_node_construct_child(cube_node* node);
  void   cube_node_free(cube_node* node);

  cube_process* cube_node_get_child(cube_node* node, int i);
  struct cube_machine* cube_node_get_parent(cube_node* node);
  char*  cube_node_get_name(cube_node* node);
  int    cube_node_num_children(cube_node* node);
  int    cube_node_get_level(cube_node* node);
  void   cube_node_writeXML(cube_node* node, FILE* fp);
  void   cube_node_set_id(cube_node* node, int new_id);
  int    cube_node_get_id(cube_node* node);
  int    cube_node_equal(cube_node* a, cube_node* b);
  void   cube_node_add_child(struct cube_machine* parent, cube_node* node);

#ifdef __cplusplus
}
#endif

#endif
