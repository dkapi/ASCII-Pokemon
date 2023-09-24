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

void dijkstra_map(terrain_map_t *map, Location_t *start, 
                dijkstra_node_t *dNode[GRID_WIDTH][GRID_HEIGHT],
                dijkstra_node_t *dn);
void print_dijkstra_map(dijkstra_node_t *dNode[GRID_WIDTH][GRID_HEIGHT]);

#endif /*DIJKSTRA_H*/