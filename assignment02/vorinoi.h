#ifndef _WORLDGEN_VORONOI_H
#define _WORLDGEN_VORONOI_H

#include "worldgen.h"

typedef struct Seed
{
    Location_t loc;
    char tile;
} Seed_t;

void generate_voronoi_map(terrain_map_t *gridMap);

void fill_vorinoi_map(terrain_map_t *gridMap, Seed_t* seeds);  // TODO: rename

void generate_map_border(terrain_map_t *gridMap);

void generate_voronoi_seeds(Seed_t* seeds);

void generate_map_gates(Gates_t* gates);

void generate_map_path(terrain_map_t *m, Gates_t gates);


#endif  /* _WORLDGEN_VORONOI_H */