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
* \file process.h
 \brief Declares types and functions to deal with process of running application.
*/
#ifndef CUBEW_PROCESS_H
#define CUBEW_PROCESS_H   

#ifdef __cplusplus
extern "C" {
#endif

  #include <stdio.h>
  #include "thread.h"

  struct cube_node;
/**
A dynamic array with processes.
*/
  typedef struct cube_parray {
    int size;
    int capacity;
    struct cube_process** data;
  } cube_parray;

/**
* Structure collection name, ID and rank of a process.
*/
  typedef struct cube_process {
    cube_tarray* child;
    struct cube_node* parent;
    char* name;
    int rank;
    int id;
  } cube_process;

  cube_process* cube_process_create(cube_process* proc);
  void   cube_process_init(cube_process* proc, char* name, int rank, struct cube_node* parent);
  void   cube_process_construct_child(cube_process* proc);
  void   cube_process_free(cube_process* proc);

  cube_thread* cube_process_get_child(cube_process* proc, int i);
  struct cube_node* cube_process_get_parent(cube_process* proc);
  int    cube_process_get_rank(cube_process* proc);
  char*  cube_process_get_name(cube_process* proc);
  int    cube_process_num_children(cube_process* proc);
  int    cube_process_get_level(cube_process* proc);
  void   cube_process_writeXML(cube_process* proc, FILE* fp);
  void   cube_process_set_id(cube_process* proc, int new_id);
  int    cube_process_get_id(cube_process* proc);
  int    cube_process_equal(cube_process* a, cube_process* b);
  void   cube_process_add_child(struct cube_node* parent, cube_process* proc);

#ifdef __cplusplus
}
#endif

#endif

