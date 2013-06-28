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
* \file thread.h
 \brief Declares types and functions to deal with threads of running application.
*/
#ifndef CUBEW_THREAD_H
#define CUBEW_THREAD_H   

#ifdef __cplusplus
extern "C" {
#endif

  #include <stdio.h>

  struct cube_process;
/**
A dynamic array of threads.
*/
  typedef struct cube_tarray {
    int size;
    int capacity;
    struct cube_thread** data;
  } cube_tarray;
/**
* A structure containing information about a thread.
*/
  typedef struct cube_thread {
    cube_tarray* child;
    struct cube_process* parent;
    char* name;
    int rank;
    int id;
  } cube_thread;

  cube_thread* cube_thread_create(cube_thread* thrd);
  void   cube_thread_init(cube_thread* thrd, char* name, int rank, struct cube_process* parent);
  void   cube_thread_construct_child(cube_thread* thrd);
  void   cube_thread_free(cube_thread* thrd);

  cube_thread* cube_thread_get_child(cube_thread* thrd, int i);
  struct cube_process* cube_thread_get_parent(cube_thread* thrd);
  char*  cube_thread_get_name(cube_thread* thrd);
  int    cube_thread_get_rank(cube_thread* thrd);
  int    cube_thread_num_children(cube_thread* thrd);
  int    cube_thread_get_level(cube_thread* thrd);
  void   cube_thread_writeXML(cube_thread* thrd, FILE* fp);
  void   cube_thread_set_id(cube_thread* thrd, int new_id);
  int    cube_thread_get_id(cube_thread* thrd);
  void   cube_thread_add_child(struct cube_process* parent, cube_thread* thrd);
  int    cube_thread_equal(cube_thread* a, cube_thread* b);

#ifdef __cplusplus
}
#endif

#endif


