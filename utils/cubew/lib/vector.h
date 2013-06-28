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
* \file vector.h
* \brief Contains macros XALLOC, ALLOC, REALLOC and ADD_NEXT for memory allocation.
*/


#ifndef CUBEW_VECTOR_H
#define CUBEW_VECTOR_H   

#ifdef __cplusplus
extern "C" {
#endif

/** Macros for handling dynamic arrays */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

extern int cubew_trace;

/** allocation with optional log message and only warning if unsuccessful */
#define ALLOC(MEMORY,NMEMB,MTYPE) MEMORY = (MTYPE *) calloc(NMEMB,sizeof(MTYPE)); \
  if (cubew_trace) fprintf(stderr,"%s: calloc(%lu,%lu) = %p\n", __func__, \
        (unsigned long) NMEMB, (unsigned long) sizeof(MTYPE), MEMORY); \
  if (!MEMORY) { fprintf(stderr, "%s: calloc(%lu,%lu): %s\n", \
        __func__, (unsigned long) NMEMB, (unsigned long) sizeof(MTYPE), \
        strerror(errno)); }

/** allocation with exit if unsuccessful */
#define XALLOC(MEMORY,NMEMB,MTYPE) MEMORY = (MTYPE *) calloc(NMEMB,sizeof(MTYPE)); \
  if (cubew_trace) fprintf(stderr,"%s: calloc(%lu,%lu) = %p\n", __func__, \
        (unsigned long) NMEMB, (unsigned long) sizeof(MTYPE), MEMORY); \
  if (!MEMORY) { fprintf(stderr, "%s: calloc(%lu,%lu): %s\n", \
        __func__, (unsigned long) NMEMB, (unsigned long) sizeof(MTYPE), \
        strerror(errno)); exit(1); }

/** (re)allocation with exit if unsuccessful */
#define REALLOC(MEMORY,CAST,MSIZE) \
  if (cubew_trace) fprintf(stderr,"%s: realloc(%p,%lu)", __func__, \
       MEMORY, (unsigned long) MSIZE); \
  MEMORY = CAST realloc(MEMORY,MSIZE); \
  if (cubew_trace) fprintf(stderr," = %p\n", MEMORY); \
  if (!MEMORY) { fprintf(stderr, "%s: realloc(%lu): %s\n", \
        __func__, (unsigned long) MSIZE, strerror(errno)); exit(2); }

/** append element to vector */
#define ADD_NEXT(VECTOR, ELEMENT, ETYPE) \
  if (VECTOR->size == VECTOR->capacity) { \
    if (VECTOR->capacity == 0) VECTOR->capacity = 1; \
    else VECTOR->capacity *= 2; \
    REALLOC(VECTOR->data, (ETYPE *), sizeof(ETYPE)*(VECTOR->capacity)); \
  } \
  VECTOR->data[VECTOR->size] = ELEMENT; \
  VECTOR->size++;

#ifdef __cplusplus
}
#endif

#endif

