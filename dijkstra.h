#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "heap.h"
#include "worldgen.h"

typedef struct node 
{
    Location_t *from, *pos, *prev;
    struct queue_node *next;
    int distance, cost;
    int visited;
    heap_node_t *hn;
}dijkstra_node_t;

dijkstra_node_t* dijkstra_map(terrain_map_t *map, Location_t *l);


#endif /*DIJKSTRA_H*/