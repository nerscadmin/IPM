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
* \file cnode.h
  \brief Declares a types and functions to deal with calee node in the cube. 
*/
#ifndef CUBEW_CNODE_H
#define CUBEW_CNODE_H

#ifdef __cplusplus
extern "C" {
#endif

  #include <stdio.h>

  struct cube_region; ///< Empty structure defines a region of the source code,
	
/**
* Synonym for array of cnodes.
*/
  typedef struct carray cube_carray;

  typedef struct cube_cnode {
    struct cube_region* callee; ///< Calee of this cnode.
    struct cube_cnode* parent;///< This cnode belongs to cnode "parents".
    cube_carray *child;///< It has some children.
    char* mod;  ///< ??????
    int line; ///< Start of the source code lines.
    int id; /// id of the cnode.
  } cube_cnode;


  cube_cnode* cube_cnode_create(cube_cnode* cnode);
  void cube_cnode_init(cube_cnode* cnode, struct cube_region* callee, char* mod, int line, cube_cnode* parent);
  void cube_cnode_construct_child(cube_cnode* cnode);
  void cube_cnode_free(cube_cnode* cnode);

  cube_cnode* cube_cnode_get_child(cube_cnode* cnode, int i);
  cube_cnode* cube_cnode_get_parent(cube_cnode* cnode);
  int    cube_cnode_get_line(cube_cnode* cnode);
  int    cube_cnode_num_children(cube_cnode* cnode);
  char*  cube_cnode_get_mod(cube_cnode* cnode);
  struct cube_region* cube_cnode_get_callee(cube_cnode* cnode);
  struct cube_region* cube_cnode_get_caller(cube_cnode* cnode);
  void   cube_cnode_writeXML(cube_cnode* cnode, FILE* fp);
  void   cube_cnode_add_child(cube_cnode* parent, cube_cnode* cnode);
  int    cube_cnode_equal(cube_cnode* a, cube_cnode* b);
  void   cube_cnode_set_id(cube_cnode* cnode, int new_id);
  int    cube_cnode_get_id(cube_cnode* cnode);
  void   cube_cnode_assign_ids(cube_cnode* cnode, int* id);
  int    cube_cnode_get_level(cube_cnode* cnode);

#ifdef __cplusplus
}
#endif

#endif


