#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <stdint.h>

#include "heap.h"
#include "worldgen.h"
#include "characters.h"

typedef struct dijk_map_t {
  Location_t pos;
  uint32_t cost;
  heap_node_t *hn;
  struct tile_s *tile;
  struct character_s *npc;
} dijk_map_t;

void dijkstra_map(terrain_map_t *map, Location_t *start,
                  dijk_map_t dNode[GRID_HEIGHT][GRID_WIDTH],
                  struct character_s *npc);

/* Fill the global hiker_dist / rival_dist maps for the given PC position. */
void compute_dist_maps(terrain_map_t *map, Location_t *start);

void print_dist_map(int32_t dist[GRID_HEIGHT][GRID_WIDTH]);

#endif /* DIJKSTRA_H */