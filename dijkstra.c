#include <stdio.h>

#include "heap.h"
#include "dijkstra.h"
#include "characters.h"
#include "tiles.h"

static int32_t character_cost_cmp(const void *key, const void *with)
{
  const dijk_map_t *k = key;
  const dijk_map_t *w = with;

  if (k->cost < w->cost) return -1;
  if (k->cost > w->cost) return 1;
  return 0;
}

void dijkstra_map(terrain_map_t *map, Location_t *start,
                  dijk_map_t dNode[GRID_HEIGHT][GRID_WIDTH],
                  struct character_s *npc)
{
  int i, j;
  dijk_map_t *dn;
  heap_t h;

  /* Re-initialize on every call so the distances are correct no matter how
   * many times (and for which grid) this is invoked. */
  for (i = 0; i < GRID_HEIGHT; i++) {
    for (j = 0; j < GRID_WIDTH; j++) {
      dNode[i][j].pos.x = i;
      dNode[i][j].pos.y = j;
      dNode[i][j].tile = char_to_tile_s(map->grid[i][j]);
      dNode[i][j].npc = npc;
      dNode[i][j].cost = INT32_MAX;
    }
  }
  dNode[start->x][start->y].cost = 0;

  heap_init(&h, character_cost_cmp, NULL);

  for (i = 0; i < GRID_HEIGHT; i++) {
    for (j = 0; j < GRID_WIDTH; j++) {
      if (npc->cost(*dNode[i][j].tile) != INT32_MAX) {
        dNode[i][j].hn = heap_insert(&h, &dNode[i][j]);
      } else {
        dNode[i][j].hn = NULL;
      }
    }
  }

  while ((dn = heap_remove_min(&h))) {
    dn->hn = NULL; /* mark finalized */

    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if (dx == 0 && dy == 0) continue;

        int nx = dn->pos.x + dx;
        int ny = dn->pos.y + dy;

        if (nx >= 0 && nx < GRID_HEIGHT && ny >= 0 && ny < GRID_WIDTH) {
          dijk_map_t *neighbor = &dNode[nx][ny];

          if (neighbor->hn) {
            /* Cost to enter the NEIGHBOR tile, not the current one. */
            uint32_t newCost = dn->cost + npc->cost(*neighbor->tile);
            if (newCost < neighbor->cost) {
              neighbor->cost = newCost;
              heap_decrease_key_no_replace(&h, neighbor->hn);
            }
          }
        }
      }
    }
  }
  heap_delete(&h);
}

void compute_dist_maps(terrain_map_t *map, Location_t *start)
{
  static dijk_map_t hmap[GRID_HEIGHT][GRID_WIDTH];
  static dijk_map_t rmap[GRID_HEIGHT][GRID_WIDTH];
  int i, j;

  dijkstra_map(map, start, hmap, &hiker);
  dijkstra_map(map, start, rmap, &rival);

  for (i = 0; i < GRID_HEIGHT; i++) {
    for (j = 0; j < GRID_WIDTH; j++) {
      hiker_dist[i][j] = (int32_t) hmap[i][j].cost;
      rival_dist[i][j] = (int32_t) rmap[i][j].cost;
    }
  }
}

void print_dist_map(int32_t dist[GRID_HEIGHT][GRID_WIDTH])
{
  int i, j;

  for (i = 0; i < GRID_HEIGHT; i++) {
    for (j = 0; j < GRID_WIDTH; j++) {
      if (dist[i][j] == INT32_MAX) {
        printf("   ");
      } else {
        printf("%02d ", dist[i][j] % 100);
      }
    }
    printf("\n");
  }
}