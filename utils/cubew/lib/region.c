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
* \file region.c
 \brief Defines types and functions to deal with regions in source code of running application.
*/
#include <stdlib.h>
#include <string.h>
#include "region.h"
#include "vector.h"

typedef struct rarray {
  int size;
  int capacity;
  struct cube_cnode** data;
} rarray;///< A dynamic array of regions.


/**
* Allocationg memory for a region.
*
*/
cube_region* cube_region_create(cube_region* this) {
  if (this == NULL) {
    ALLOC(this, 1, cube_region);
  }
  if (this != NULL) cube_region_construct_cnode(this);
  return this;
}

/**
* Filling a region with the information.
*
*/
void cube_region_init(cube_region* this, char* name, int begln, int endln, 
                      char* url, char* descr, char* mod) {
  this->name = name;
  this->begln = begln;
  this->endln = endln;
  this->url = url;
  this->descr = descr;
  this->mod = mod;
}

/**
* Creating of callee node to this region.
*
*/
void cube_region_construct_cnode(cube_region* this) {
  XALLOC(this->cnode, 1, cube_rarray);
  this->cnode->size = 0;
  this->cnode->capacity = 0;
}

/**
* Addition of the callee node to this region.
*
*/
void cube_region_add_cnode(cube_region* this, cube_cnode* cnode) {
  int i = 0;
  int size = cube_region_num_children(this);
  for (i = 0; i < size; i++) {
    if (cube_cnode_equal(cnode, this->cnode->data[i]) == 1) return;
  }
  ADD_NEXT(this->cnode, cnode, cube_cnode *);
}

/**
* Releases memory for a region.
*
*/
void cube_region_free(cube_region* this) {
  free(this->cnode->data);
  free(this->cnode);
  if (this != NULL) free(this);
}

char* cube_region_get_name(cube_region* this) {
  return this->name;
}

char* cube_region_get_url(cube_region* this) {
  return this->url;
}

char* cube_region_get_descr(cube_region* this) {
  return this->descr;
}

char* cube_region_get_mod(cube_region* this) {
  return this->mod;
}

int cube_region_get_begn_ln(cube_region* this) {
  return (this->begln <= 0) ? -1 : this->begln;
}

int cube_region_get_end_ln(cube_region* this) {
  return (this->endln <= 0) ? -1 : this->endln;
}

int cube_region_num_children(cube_region* this) {
  return this->cnode->size;
}

/**
* Writes XML output for region in to .cube file.
* 
* A special algorithm for saving a name of the region implemented.
*/
void cube_region_writeXML(cube_region* this, FILE* fp) {
  char* r = NULL;
  char* s = cube_region_get_name(this);
  int len = strlen(s);
  unsigned toks=0;
  char* p=s;
  /* determine number of tokens to convert */
  while (p <= s+len) {
    size_t n=strcspn(p, "<>&"); /* strcspn(p, "<>&\'\""); */
    p+=(n+1);
    toks++;
  }
  if (toks) r=calloc(len+(toks*4),1); /* allocate sufficiently long string */
  p=s;
  /* replace each token in new string */
  if (r) while (p <= s+len) {
    size_t n=strcspn(p, "<>&"); /* strcspn(p, "<>&\'\""); */
    strncat(r, p, n);
    p+=n;
    switch(*p) {
      case '<': strcat(r,"lt;"); break;
      case '>': strcat(r,"gt;"); break;
      case '&': strcat(r,"amp;"); break;
#if 0 /* these don't seem required for now */
      case '\'': strcat(r,"apos;"); break;
      case '\"': strcat(r,"quot;"); break;
#endif
      default : break;
    }
    p++;
  }

  fprintf(fp, "    <region id=\"%d\" mod=\"%s\" begin=\"%d\" end=\"%d\">\n",
                   cube_region_get_id(this), cube_region_get_mod(this), 
                   cube_region_get_begn_ln(this), cube_region_get_end_ln(this));
  fprintf(fp, "      <name>%s</name>\n", r ? r : s);
  fprintf(fp, "      <url>%s</url>\n", cube_region_get_url(this));
  fprintf(fp, "      <descr>%s</descr>\n", cube_region_get_descr(this));
  fprintf(fp, "    </region>\n");

  if (r) free(r);
}

/**
* Compares equaliy of two regions
*/

int cube_region_equal(cube_region* a, cube_region* b) {
  if (strcmp(cube_region_get_name(a), cube_region_get_name(b)) == 0) {
    if (strcmp(cube_region_get_mod(a), cube_region_get_mod(b)) == 0) 
      return 1;
  }
  return 0;
} 

void cube_region_set_id(cube_region* this, int new_id) {
  this->id = new_id;
}

int cube_region_get_id(cube_region* this) {
  return this->id;
}
