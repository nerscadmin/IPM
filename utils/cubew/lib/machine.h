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
* \file machine.h
* \brief Declares types and functions to deal with running machine as whole object.
*/
#ifndef CUBEW_MACHINE_H
#define CUBEW_MACHINE_H   

#ifdef __cplusplus
extern "C" {
#endif

  #include <stdio.h>
  #include "node.h"

  struct cube_node;
/**
* A dynamical array containing information about machine and its children.
*/
  typedef struct cube_machine {
    cube_narray* child; 
    char* name;
    char* desc;
    int id;
  } cube_machine;

  cube_machine* cube_machine_create(cube_machine* mach);
  void   cube_machine_init(cube_machine* mach, char* name, char* desc);
  void   cube_machine_construct_child(cube_machine* mach);
  void   cube_machine_reserve_nodes(cube_machine* mach, unsigned num);
  void   cube_machine_free(cube_machine* mach);

  struct cube_node* cube_machine_get_child(cube_machine* mach, int i);
  char*  cube_machine_get_name(cube_machine* mach);
  char*  cube_machine_get_desc(cube_machine* mach);
  int    cube_machine_num_children(cube_machine* mach);
  int    cube_machine_get_level(cube_machine* mach);
  void   cube_machine_writeXML(cube_machine* mach, FILE* fp);
  void   cube_machine_set_id(cube_machine* mach, int new_id);
  int    cube_machine_get_id(cube_machine* mach);
  int    cube_machine_equal(cube_machine* a, cube_machine* b);

#ifdef __cplusplus
}
#endif

#endif
