#include "building.h"
#include "tiles.h"

void generate_building_interior(terrain_map_t *map, int type)
{
  int x, y;

  /* Border walls + interior floor. */
  for (x = 0; x < GRID_HEIGHT; x++)
    for (y = 0; y < GRID_WIDTH; y++)
      map->grid[x][y] = mountainTile.ascii;
  for (x = 1; x < GRID_HEIGHT - 1; x++)
    for (y = 1; y < GRID_WIDTH - 1; y++)
      map->grid[x][y] = clearingTile.ascii;

  /* Potted plants in the top corners. */
  map->grid[1][2] = '*';
  map->grid[1][GRID_WIDTH - 3] = '*';

  if (type == BUILDING_CENTER) {
    /* Nurse Joy (heal) on the left, the PC on the right, mid-room. */
    map->grid[10][10] = BUILDING_NURSE;
    map->grid[10][30] = BUILDING_PC;
    for (y = 8; y <= 12; y++)  map->grid[11][y] = '=';   /* counter */
    for (y = 28; y <= 32; y++) map->grid[11][y] = '=';   /* PC desk */
  } else {
    /* The shop clerk behind a counter, mid-room. */
    map->grid[10][10] = BUILDING_CLERK;
    for (y = 8; y <= 12; y++) map->grid[11][y] = '=';
  }

  /* A doormat just inside the door. */
  for (y = GRID_WIDTH / 2 - 3; y <= GRID_WIDTH / 2 + 3; y++) {
    map->grid[GRID_HEIGHT - 4][y] = '=';
  }

  /* The door at the bottom center. */
  map->grid[GRID_HEIGHT - 2][GRID_WIDTH / 2] = BUILDING_DOOR;
}