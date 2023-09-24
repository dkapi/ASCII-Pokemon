#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "heap.h"


typedef struct queue_node {
    int x, y;
    struct queue_node *next;
    
} queue_node_t;


#endif /*DIJKSTRA_H*/