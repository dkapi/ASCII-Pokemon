#ifndef _WORLDGEN_VORONOI_H
#define _WORLDGEN_VORONOI_H

#include "worldgen.h"

typedef struct Seed
{
    Location_t loc;
    char tile;
} Seed_t;

void generate_voronoi_map(terrain_map_t *map, Gates_t gates);

void fill_vorinoi_map(terrain_map_t *map, Seed_t* seeds);  // TODO: rename

void generate_map_border(terrain_map_t *map);

void generate_voronoi_seeds(Seed_t* seeds);

void generate_map_gates(terrain_map_t *map, Gates_t* gates);

void generate_map_path(terrain_map_t *m, Gates_t gates);

void find_building_location(terrain_map_t *m, Location_t *l);

void place_center(terrain_map_t *m);

void place_mart(terrain_map_t *m);

#endif  /* _WORLDGEN_VORONOI_H */