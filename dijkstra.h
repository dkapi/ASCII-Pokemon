#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "worldgen.h"
#include "characters.h"


void dijkstra_map(terrain_map_t *map, Location_t *start, 
                dijk_map_t dNode[GRID_HEIGHT][GRID_WIDTH],
                struct character_s *npc);
void print_dijkstra_map(dijk_map_t dNode[GRID_HEIGHT][GRID_WIDTH]);

#endif /*DIJKSTRA_H*/