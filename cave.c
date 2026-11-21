#include <stdlib.h>

#include "cave.h"
#include "tiles.h"
#include "voronoi.h"

/* Count wall cells in the 8-neighborhood; out-of-bounds (border) counts as wall. */
static int wall_neighbors(const terrain_map_t *m, int x, int y)
{
  int n = 0;
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      if (!dx && !dy) continue;
      int nx = x + dx, ny = y + dy;
      if (nx < 0 || nx >= GRID_HEIGHT || ny < 0 || ny >= GRID_WIDTH ||
          m->grid[nx][ny] == CAVE_WALL) {
        n++;
      }
    }
  }
  return n;
}

/* Iterative 4-way flood fill of non-wall cells; reached cells are marked. */
static void flood(const terrain_map_t *m, char seen[GRID_HEIGHT][GRID_WIDTH],
                  int sx, int sy)
{
  static int qx[(GRID_HEIGHT) * (GRID_WIDTH)];
  static int qy[(GRID_HEIGHT) * (GRID_WIDTH)];
  static const int dx[4] = { 1, -1, 0, 0 };
  static const int dy[4] = { 0, 0, 1, -1 };
  int head = 0, tail = 0;

  qx[tail] = sx; qy[tail] = sy; tail++;
  seen[sx][sy] = 1;

  while (head < tail) {
    int x = qx[head], y = qy[head]; head++;
    for (int d = 0; d < 4; d++) {
      int nx = x + dx[d], ny = y + dy[d];
      if (nx < 1 || nx >= GRID_HEIGHT - 1 || ny < 1 || ny >= GRID_WIDTH - 1) continue;
      if (seen[nx][ny] || m->grid[nx][ny] == CAVE_WALL) continue;
      seen[nx][ny] = 1;
      qx[tail] = nx; qy[tail] = ny; tail++;
    }
  }
}

void generate_cave_map(terrain_map_t *m, Gates_t gates)
{
  int x, y;

  /* 1. Full border + random interior fill (~45% wall). */
  for (x = 0; x < GRID_HEIGHT; x++)
    for (y = 0; y < GRID_WIDTH; y++)
      m->grid[x][y] = CAVE_WALL;
  for (x = 1; x < GRID_HEIGHT - 1; x++)
    for (y = 1; y < GRID_WIDTH - 1; y++)
      m->grid[x][y] = (rand() % 100 < 45) ? CAVE_WALL : CAVE_FLOOR;

  /* 2. Cellular-automata smoothing (4-5 rule). */
  for (int pass = 0; pass < 4; pass++) {
    static char buf[GRID_HEIGHT][GRID_WIDTH];
    for (x = 1; x < GRID_HEIGHT - 1; x++)
      for (y = 1; y < GRID_WIDTH - 1; y++)
        buf[x][y] = (wall_neighbors(m, x, y) >= 5) ? CAVE_WALL : CAVE_FLOOR;
    for (x = 1; x < GRID_HEIGHT - 1; x++)
      for (y = 1; y < GRID_WIDTH - 1; y++)
        m->grid[x][y] = buf[x][y];
  }

  /* 3. Tunnel gates (sideways cave network).  Carried gates stay put; the
   * rest are randomized.  Carve a floor alcove just inside each gate so it
   * joins the cavern. */
  generate_map_gates(m, &gates);
  m->gates = gates;
  m->grid[gates.left.x][1]                        = CAVE_FLOOR;
  m->grid[gates.right.x][GRID_WIDTH - 2]          = CAVE_FLOOR;
  m->grid[1][gates.top.y]                         = CAVE_FLOOR;
  m->grid[GRID_HEIGHT - 2][gates.bottom.y]        = CAVE_FLOOR;

  /* Gates leading off the edge of the world have no neighbour cave. */
  if (m->location.x == 0)   m->grid[gates.left.x][gates.left.y]     = CAVE_WALL;
  if (m->location.x == 400) m->grid[gates.right.x][gates.right.y]   = CAVE_WALL;
  if (m->location.y == 0)   m->grid[gates.bottom.x][gates.bottom.y] = CAVE_WALL;
  if (m->location.y == 400) m->grid[gates.top.x][gates.top.y]       = CAVE_WALL;

  /* 4. Connectivity: flood the anchor region, then carve a straight floor
   *    tunnel from every disconnected floor cell back to it. */
  char seen[GRID_HEIGHT][GRID_WIDTH] = { 0 };
  int ax = 1, ay = 1;
  int found = 0;
  for (x = 1; x < GRID_HEIGHT - 1 && !found; x++)
    for (y = 1; y < GRID_WIDTH - 1 && !found; y++)
      if (m->grid[x][y] == CAVE_FLOOR) { ax = x; ay = y; found = 1; }

  if (found) {
    flood(m, seen, ax, ay);

    for (x = 1; x < GRID_HEIGHT - 1; x++) {
      for (y = 1; y < GRID_WIDTH - 1; y++) {
        if (m->grid[x][y] == CAVE_FLOOR && !seen[x][y]) {
          int cx = x, cy = y;
          while (!(seen[cx][cy])) {
            m->grid[cx][cy] = CAVE_FLOOR;
            seen[cx][cy] = 1;
            if (cx > ax) cx--;
            else if (cx < ax) cx++;
            else if (cy > ay) cy--;
            else if (cy < ay) cy++;
            else break;
          }
        }
      }
    }
  }

  /* 5. Place the exit stair and a few loot pickups on floor cells. */
  static int floors[(GRID_HEIGHT) * (GRID_WIDTH)][2];
  int nf = 0;
  for (x = 1; x < GRID_HEIGHT - 1; x++)
    for (y = 1; y < GRID_WIDTH - 1; y++)
      if (m->grid[x][y] == CAVE_FLOOR) {
        floors[nf][0] = x; floors[nf][1] = y; nf++;
      }

  if (nf > 0) {
    int e = rand() % nf;
    m->grid[floors[e][0]][floors[e][1]] = CAVE_EXIT;
  }
  for (int i = 0; i < 3 && nf > 0; i++) {
    int k = rand() % nf;
    m->grid[floors[k][0]][floors[k][1]] = (rand() & 1) ? CAVE_MONEY : CAVE_ITEM;
  }
}