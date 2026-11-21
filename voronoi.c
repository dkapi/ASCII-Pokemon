#include <stdlib.h>
#include <stdbool.h>

#include "voronoi.h"
#include "tiles.h"
#include "noise.h"

/* Selected by --noise: value-noise biomes instead of Voronoi regions. */
bool terrain_noise = false;

/* Squared distance is enough for nearest-seed comparisons and avoids the
 * sqrt() per cell (and the -lm link dependency). */
static inline int dist_sq(int x, int y, int x2, int y2)
{
  int dx = x - x2;
  int dy = y - y2;
  return dx * dx + dy * dy;
}

void generate_voronoi_map(terrain_map_t *map, Gates_t gates)
{
  generate_map_border(map);

  if (terrain_noise) {
    fill_noise_map(map);
  } else {
    Seed_t seeds[SEED_NUM] = { 0 };
    generate_voronoi_seeds(&seeds[0]);
    fill_vorinoi_map(map, &seeds[0]);
  }

  generate_map_gates(map, &gates);
  map->gates = gates;
  generate_map_path(map, gates);
  place_center(map);
  place_mart(map);
}

void generate_map_border(terrain_map_t *gridMap)
{
  int i, j;

  for (i = 0; i < GRID_HEIGHT; i++) {
    gridMap->grid[i][0] = mountainTile.ascii;
    gridMap->grid[i][GRID_WIDTH - 1] = mountainTile.ascii;
  }

  for (j = 0; j < GRID_WIDTH; j++) {
    gridMap->grid[0][j] = mountainTile.ascii;
    gridMap->grid[GRID_HEIGHT - 1][j] = mountainTile.ascii;
  }

  for (i = 1; i < GRID_HEIGHT - 1; i++) {
    for (j = 1; j < GRID_WIDTH - 1; j++) {
      gridMap->grid[i][j] = clearingTile.ascii;
    }
  }
}

void generate_voronoi_seeds(Seed_t *seeds)
{
  /* Biome tiles that can seed a region (everything except Path). */
  static const enum tile_e biomes[] = {
    TreeTile, MountainTile, TallGrass, Clearing, Water,
  };
  const int num_biomes = (int) (sizeof(biomes) / sizeof(biomes[0]));

  for (int i = 0; i < SEED_NUM; i++) {
    seeds[i].loc.x = 1 + (rand() % (GRID_HEIGHT - 1));
    seeds[i].loc.y = 1 + (rand() % (GRID_WIDTH - 1));
    seeds[i].tile = tiles[biomes[i % num_biomes]]->ascii;
  }
}

void fill_vorinoi_map(terrain_map_t *gridMap, Seed_t *seeds)
{
  int x, y, i;

  /* start at one because of border */
  for (x = 1; x < GRID_HEIGHT - 1; x++) {
    for (y = 1; y < GRID_WIDTH - 1; y++) {
      int closestSeed = 0;
      int closestDist = dist_sq(x, y, seeds[0].loc.x, seeds[0].loc.y);

      for (i = 1; i < SEED_NUM; i++) {
        int currDist = dist_sq(x, y, seeds[i].loc.x, seeds[i].loc.y);
        if (currDist < closestDist) {
          closestSeed = i;
          closestDist = currDist;
        }
      }

      if (seeds[closestSeed].tile == treeTile.ascii) {
        int treeChance = rand() % 100;
        gridMap->grid[x][y] = (treeChance < 25) ? treeTile.ascii
                                                : clearingTile.ascii;
      } else {
        gridMap->grid[x][y] = seeds[closestSeed].tile;
      }
    }
  }
}

/* ---- value-noise biome fill (--noise) ----
 * Elevation + moisture, thresholded into terrain bands and seeded by WORLD
 * coordinates so a lake or forest spans maps without a seam.  The route and
 * buildings are placed afterward exactly as with the Voronoi fill, so the
 * connectivity invariants are unchanged. */
void fill_noise_map(terrain_map_t *m)
{
  double ew0 = (double) m->location.x * GRID_WIDTH;
  double ns0 = (double) m->location.y * GRID_HEIGHT;

  for (int x = 1; x < GRID_HEIGHT - 1; x++) {
    for (int y = 1; y < GRID_WIDTH - 1; y++) {
      double ew = ew0 + (double) y;   /* east-west, continuous across maps */
      double ns = ns0 + (double) x;   /* north-south, continuous across maps */

      double e = fbm2(ew * 0.035, ns * 0.035, 4);              /* elevation */
      double w = fbm2(ew * 0.085 + 137.0, ns * 0.085, 3);      /* moisture */

      char t;
      if (e < -0.28)      t = waterTile.ascii;
      else if (e < -0.14) t = clearingTile.ascii;       /* shore */
      else if (e < 0.02)  t = (w > 0.0) ? tallGrassTile.ascii : clearingTile.ascii;
      else if (e < 0.26)  t = (w > -0.10) ? treeTile.ascii : tallGrassTile.ascii;
      else if (e < 0.44)  t = (w > 0.05) ? treeTile.ascii : clearingTile.ascii;
      else                t = mountainTile.ascii;

      m->grid[x][y] = t;
    }
  }
}

void generate_map_gates(terrain_map_t *map, Gates_t *gates)
{
  /* x is the row and y the column within the grid. */
  if (gates->left.x == 0 && gates->left.y == 0) {
    gates->left.x = 1 + (rand() % (GRID_HEIGHT - 2));
    gates->left.y = 0;
  }
  if (gates->right.x == 0 && gates->right.y == 0) {
    gates->right.x = 1 + (rand() % (GRID_HEIGHT - 2));
    gates->right.y = GRID_WIDTH - 1;
  }
  if (gates->bottom.x == 0 && gates->bottom.y == 0) {
    gates->bottom.x = GRID_HEIGHT - 1;
    gates->bottom.y = 1 + (rand() % (GRID_WIDTH - 2));
  }
  if (gates->top.x == 0 && gates->top.y == 0) {
    gates->top.x = 0;
    gates->top.y = 1 + (rand() % (GRID_WIDTH - 2));
  }
  map->grid[gates->left.x][gates->left.y]     = pathTile.ascii;
  map->grid[gates->right.x][gates->right.y]   = pathTile.ascii;
  map->grid[gates->bottom.x][gates->bottom.y] = pathTile.ascii;
  map->grid[gates->top.x][gates->top.y]       = pathTile.ascii;
}

void generate_map_path(terrain_map_t *m, Gates_t gates)
{
  int direction;

  /* left gate: go right until bottom gate's row */
  while (gates.left.y < gates.bottom.y) {
    m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
    gates.left.y++;
  }

  /* left gate: move vertically toward the right gate's row */
  direction = gates.left.x > gates.right.x ? -1 : 1;
  while (gates.left.x != gates.right.x) {
    m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
    gates.left.x += direction;
  }

  /* left gate: finish horizontally to the right gate */
  while (gates.left.y != GRID_WIDTH) {
    m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
    gates.left.y++;
  }

  /* bottom gate: move up to the right gate's row */
  while (gates.bottom.x > gates.left.x) {
    m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
    gates.bottom.x--;
  }

  /* bottom gate: move horizontally toward the top gate */
  direction = gates.bottom.y > gates.top.y ? -1 : 1;
  while (gates.bottom.y != gates.top.y) {
    m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
    gates.bottom.y += direction;
  }

  /* bottom gate: finish moving up to the top gate */
  while (gates.bottom.x != 0) {
    m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
    gates.bottom.x--;
  }

  /* A gate leading off the edge of the world has no neighbour map, so
   * block it with mountain.  location.x is east-west, location.y is
   * north-south (increasing = north).
   *
   * Use m->gates (the pristine positions saved up in generate_voronoi_map)
   * rather than the local `gates`, whose fields the drawing loops above
   * have mutated -- e.g. left.y is left at GRID_WIDTH (out of bounds). */
  if (m->location.y == 400) {
    m->grid[m->gates.top.x][m->gates.top.y] = mountainTile.ascii;
  }
  if (m->location.x == 0) {
    m->grid[m->gates.left.x][m->gates.left.y] = mountainTile.ascii;
  }
  if (m->location.y == 0) {
    m->grid[m->gates.bottom.x][m->gates.bottom.y] = mountainTile.ascii;
  }
  if (m->location.x == 400) {
    m->grid[m->gates.right.x][m->gates.right.y] = mountainTile.ascii;
  }
}

bool find_building_location(terrain_map_t *m, Location_t *l)
{
  for (int attempts = 0; attempts < 1000; attempts++) {
    l->x = rand() % (GRID_HEIGHT - 3) + 1;
    l->y = rand() % (GRID_WIDTH - 3) + 1;

    /* The 2x2 building must be reachable: adjacent to a path on one side. */
    bool adjacent = false;
    if ((m->grid[l->x - 1][l->y]     == pathTile.ascii &&
         m->grid[l->x - 1][l->y + 1] == pathTile.ascii) ||
        (m->grid[l->x + 2][l->y]     == pathTile.ascii &&
         m->grid[l->x + 2][l->y + 1] == pathTile.ascii) ||
        (m->grid[l->x][l->y - 1]     == pathTile.ascii &&
         m->grid[l->x + 1][l->y - 1] == pathTile.ascii) ||
        (m->grid[l->x][l->y + 2]     == pathTile.ascii &&
         m->grid[l->x + 1][l->y + 2] == pathTile.ascii)) {
      adjacent = true;
    }
    if (!adjacent) {
      continue;
    }

    /* And its four cells must be free of paths and other buildings. */
    bool clear = true;
    for (int dx = 0; dx <= 1 && clear; dx++) {
      for (int dy = 0; dy <= 1; dy++) {
        char c = m->grid[l->x + dx][l->y + dy];
        if (c == pathTile.ascii || c == pokeMart.ascii ||
            c == pokemonCenter.ascii) {
          clear = false;
          break;
        }
      }
    }
    if (clear) {
      return true;
    }
  }

  return false;
}

static void place_building(terrain_map_t *m, char ascii)
{
  /* Spawn probability falls off with Manhattan distance from world center
   * (200, 200).  Always place on the center map. */
  int d = abs(200 - m->location.x) + abs(200 - m->location.y);
  int p = d > 200 ? 5 : (50 - ((45 * d) / 200));

  if (!d || (rand() % 100) < p) {
    Location_t l;
    if (find_building_location(m, &l)) {
      m->grid[l.x][l.y]             = ascii;
      m->grid[l.x + 1][l.y]         = ascii;
      m->grid[l.x][l.y + 1]         = ascii;
      m->grid[l.x + 1][l.y + 1]     = ascii;
    }
  }
}

void place_mart(terrain_map_t *m)
{
  place_building(m, pokeMart.ascii);
}

void place_center(terrain_map_t *m)
{
  place_building(m, pokemonCenter.ascii);
}