/* Version 4.0. (c) Copyright 1993-2013 by the University of Washington.
   Written by Joseph Felsenstein, Akiko Fuseki, Sean Lamont, and Andrew Keeffe.
   Permission is granted to copy and use this program provided no fee is
   charged for it and provided that this copyright notice is not removed. */


/*
    dist.h: included in fitch, kitsch, & neighbor
*/


#define over            60


typedef long *intvector;

typedef node **pointptr;

typedef struct dist_node {
  node node;            /* Base object, must be first */
  vector d, w;
  double t;
  boolean sametime;
  double dist;                   /* dist used in fitch   contml*/
} dist_node;

#ifndef OLDC
/*function prototypes*/
void alloctree(pointptr *, long);
void freetree(pointptr *, long);
void allocd(long, pointptr);
void freed(long, pointptr);
void allocw(long, pointptr);
void freew(long, pointptr);
void setuptree(tree *, long);
void inputdata(boolean, boolean, boolean, boolean, vector *, intvector *);
void coordinates(node *, double, long *, double *, node *);
void drawline(long, double, node *, boolean);
void printree(node *, boolean, boolean);
void treeoutr(node *, long *, tree *);
void treeout(node *, long *, double, boolean, node *);
node* dist_node_new(node_type type, long index);
void dist_node_init(node* n, node_type type, long index);
void dist_node_copy(node* src, node* dst);
void dist_node_free(node **np);
/*function prototypes*/
#endif


// End.
