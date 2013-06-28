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
* \file metric.h
  \brief Declares  types and functions to deal with metrics.
*/
#ifndef CUBEW_METRIC_H
#define CUBEW_METRIC_H   

#ifdef __cplusplus
extern "C" {
#endif
 
  #include <stdio.h>

  typedef struct marray cube_marray;///< Synonim of an array for using it as an array of metrics.
/**
* A Structure of a metric.
*/

  typedef struct cube_metric {
    char* disp_name;
    char* uniq_name;
    char* dtype;
    char* uom;
    char* val;
    char* url;
    char* descr;
    int id;
    cube_marray* child;
    struct cube_metric* parent;
  } cube_metric;


  cube_metric* cube_metric_create(cube_metric* metric);
  void  cube_metric_init(cube_metric* metric, char* disp_name, char* uniq_name, char* dtype, 
                         char* uom, char* val, char* url, char* descr, cube_metric* parent);
  void  cube_metric_construct_child(cube_metric* metric);
  void  cube_metric_free(cube_metric* metric);

  cube_metric* cube_metric_get_child(cube_metric* metric, int i);
  cube_metric* cube_metric_get_parent(cube_metric* metric);
  char* cube_metric_get_disp_name(cube_metric* metric);
  char* cube_metric_get_uniq_name(cube_metric* metric);
  char* cube_metric_get_dtype(cube_metric* metric);
  char* cube_metric_get_uom(cube_metric* metric);
  char* cube_metric_get_val(cube_metric* metric);
  char* cube_metric_get_descr(cube_metric* metric);
  char* cube_metric_get_url(cube_metric* metric);
  int   cube_metric_num_children(cube_metric* metric);
  void  cube_metric_add_child(cube_metric* parent, cube_metric* met);
  void  cube_metric_assign_ids(cube_metric* metric, int* id);
  int   cube_metric_get_level(cube_metric* metric);
  void  cube_metric_writeXML(cube_metric* metric, FILE* fp);
  void  cube_metric_writeXML_data(cube_metric* metric, FILE* fp);
  void  cube_metric_set_id(cube_metric* metric, int new_id);
  void  cube_metric_set_uniq_name(cube_metric* metric, char* uniq_name);
  int   cube_metric_get_id(cube_metric* metric);           
  int   cube_metric_equal(cube_metric* a, cube_metric* b);            

#ifdef __cplusplus
}
#endif

#endif










































