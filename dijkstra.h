#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "heap.h"
#include "worldgen.h"
#include "characters.h"



typedef struct dijk_map_t
{
     Location_t pos;
    uint32_t cost;
    int visited;
    heap_node_t *hn;
     struct tile_s *tile;
    struct character_s *npc;
}dijk_map_t;

void dijkstra_map(terrain_map_t *map, Location_t *start, 
                dijk_map_t *dNode[GRID_HEIGHT][GRID_WIDTH],
                struct character_s *npc);
void print_dijkstra_map(dijk_map_t *dNode[GRID_HEIGHT][GRID_WIDTH]);
void dijkstra_free(dijk_map_t *dNode[GRID_HEIGHT][GRID_WIDTH]);


#endif /*DIJKSTRA_H*/