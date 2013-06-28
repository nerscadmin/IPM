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
* \file region.h
 \brief Declares types and functions to deal with regions in source code of running application.
*/
#ifndef CUBEW_REGION_H
#define CUBEW_REGION_H

#ifdef __cplusplus
extern "C" {
#endif
  
  #include <stdio.h>
  #include "cnode.h"

  typedef struct rarray cube_rarray; ///< A synonym of the arrays containing only regions.

/**
* A structure collecting information about a region: Start line, end line, description, url, name and so on.
*/
  typedef struct cube_region {  
    char* name;
    int begln;
    int endln;
    char* url;
    char* descr;
    char* mod;
    int id;
    cube_rarray* cnode; 
  } cube_region;


  cube_region* cube_region_create(cube_region* reg);
  void  cube_region_init(cube_region* reg, char* name, int begln, int endln, 
                         char* url, char* descr, char* mod);
  void  cube_region_construct_cnode(cube_region* reg);
  void  cube_region_free(cube_region* reg);

  char* cube_region_get_name(cube_region* reg);
  char* cube_region_get_url(cube_region* reg);
  char* cube_region_get_descr(cube_region* reg);
  char* cube_region_get_mod(cube_region* reg);
  int   cube_region_get_begn_ln(cube_region* reg);
  int   cube_region_get_end_ln(cube_region* reg);
  int   cube_region_num_children(cube_region* reg);
  void  cube_region_add_cnode(cube_region* reg, cube_cnode* cnode);
  void  cube_region_writeXML(cube_region* reg, FILE* fp);
  int   cube_region_equal(cube_region* a, cube_region* b);
  void  cube_region_set_id(cube_region* reg, int new_id);
  int   cube_region_get_id(cube_region* reg);

#ifdef __cplusplus
}
#endif

#endif
