#ifndef _WORLDGEN_H_
#define _WORLDGEN_H_

#include <stdint.h>
#include <stdlib.h>

#define GRID_HEIGHT 24 - 3
#define GRID_WIDTH 80
#define WORLD_HEIGHT 401
#define WORLD_WIDTH 401
#define WORLD_SIZE 401
#define SEED_NUM 18

typedef struct Location {
  int x;
  int y;
} Location_t;

typedef struct Gates {
  Location_t left;
  Location_t right;
  Location_t bottom;
  Location_t top;
} Gates_t;

struct character_s;

typedef struct terrain_map {
  char grid[GRID_HEIGHT][GRID_WIDTH];
  struct character_s *cmap[GRID_HEIGHT][GRID_WIDTH];
  Location_t location;
  Gates_t gates;
  int visited, cost, distance;
} terrain_map_t;

#endif /* _WORLDGEN_H_ */