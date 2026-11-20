#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "characters.h"

/* Distance-to-PC cost maps, filled by compute_dist_maps() in dijkstra.c. */
int32_t hiker_dist[GRID_HEIGHT][GRID_WIDTH];
int32_t rival_dist[GRID_HEIGHT][GRID_WIDTH];

struct character_s pc = {
  .ascii = '@',
  .mtype = MOVE_PC,
  .ctype = CHAR_PC,
  .cost = pc_cost,
};

struct character_s hiker = {
  .ascii = 'h',
  .mtype = MOVE_HIKER,
  .ctype = CHAR_HIKER,
  .cost = hiker_cost,
};

struct character_s rival = {
  .ascii = 'r',
  .mtype = MOVE_RIVAL,
  .ctype = CHAR_RIVAL,
  .cost = rival_cost,
};

struct character_s pacer = {
  .ascii = 'p',
  .direction = 1,
  .mtype = MOVE_PACER,
  .ctype = CHAR_OTHER,
  .cost = other_cost,
};

struct character_s sentry = {
  .ascii = 's',
  .mtype = MOVE_SENTRY,
  .ctype = CHAR_OTHER,
  .cost = other_cost,
};

struct character_s wanderer = {
  .ascii = 'w',
  .mtype = MOVE_WANDERER,
  .ctype = CHAR_OTHER,
  .cost = other_cost,
};

struct character_s explorer = {
  .ascii = 'e',
  .mtype = MOVE_EXPLORER,
  .ctype = CHAR_OTHER,
  .cost = other_cost,
};

struct character_s swimmer = {
  .ascii = 'm',
  .mtype = MOVE_SWIMMER,
  .ctype = CHAR_SWIMMER,
  .cost = swimmer_cost,
};

/* Every NPC, in movement/render order. */
static struct character_s *npc_list[] = {
  &hiker, &rival, &pacer, &sentry, &wanderer, &explorer, &swimmer,
};
#define NUM_NPCS ((int) (sizeof(npc_list) / sizeof(npc_list[0])))

/* ------------------------------------------------------------------------- *
 * Movement costs per terrain, per character category.                       *
 * ------------------------------------------------------------------------- */
uint32_t pc_cost(struct tile_s tile)
{
  switch (tile.tile_idx) {
    case Path:
    case PokeCenter:
    case PokeMart:
    case Clearing:
      return 10;
    case TallGrass:
      return 20;
    default:
      return INT32_MAX;
  }
}

uint32_t hiker_cost(struct tile_s tile)
{
  switch (tile.tile_idx) {
    case Path:       return 10;
    case Clearing:   return 10;
    case PokeCenter: return 50;
    case PokeMart:   return 50;
    case MountainTile: return 15;
    case TreeTile:     return 15;
    case TallGrass:    return 15;
    default:
      return INT32_MAX;
  }
}

uint32_t rival_cost(struct tile_s tile)
{
  switch (tile.tile_idx) {
    case Path:       return 10;
    case Clearing:   return 10;
    case PokeCenter: return 50;
    case PokeMart:   return 50;
    case TallGrass:  return 20;
    default:
      return INT32_MAX;
  }
}

/* Pacer/wanderer/sentry/explorer all share the rival movement profile. */
uint32_t other_cost(struct tile_s tile)
{
  return rival_cost(tile);
}

uint32_t swimmer_cost(struct tile_s tile)
{
  switch (tile.tile_idx) {
    case Water: return 7;
    default:    return INT32_MAX;
  }
}

/* ------------------------------------------------------------------------- *
 * Placement.                                                                *
 * ------------------------------------------------------------------------- */
void clear_characters(terrain_map_t *map)
{
  memset(map->cmap, 0, sizeof map->cmap);
}

void place_pc(terrain_map_t *map)
{
  Location_t l;
  int attempts = 0;

  do {
    l.x = 1 + rand() % (GRID_HEIGHT - 3);
    l.y = 1 + rand() % (GRID_WIDTH - 3);
  } while (map->grid[l.x][l.y] != pathTile.ascii && ++attempts < 10000);

  map->cmap[l.x][l.y] = &pc;
  pc.location.x = l.x;
  pc.location.y = l.y;
}

/* Pick a free, passable cell for `c` (water-only for swimmers, since their
 * cost is only finite on water). */
static Location_t random_spawn(terrain_map_t *map, struct character_s *c)
{
  Location_t l;
  int attempts = 0;

  do {
    l.x = 1 + rand() % (GRID_HEIGHT - 3);
    l.y = 1 + rand() % (GRID_WIDTH - 3);
    if (++attempts > 10000) {
      break;
    }
  } while (map->cmap[l.x][l.y] ||
           c->cost(*char_to_tile_s(map->grid[l.x][l.y])) == INT32_MAX);

  return l;
}

void place_all_npcs(terrain_map_t *map)
{
  for (int i = 0; i < NUM_NPCS; i++) {
    struct character_s *c = npc_list[i];
    Location_t l = random_spawn(map, c);

    c->location = l;
    c->travel_cost = 0;
    c->defeated = 0;
    c->direction = (rand() & 1) ? 1 : -1;
    c->cardinal = rand() & 1;
    map->cmap[l.x][l.y] = c;
  }
}

/* ------------------------------------------------------------------------- *
 * Movement: each function advances its NPC by one step.                     *
 * ------------------------------------------------------------------------- */
static bool in_bounds(int x, int y)
{
  return x > 0 && x < GRID_HEIGHT - 1 && y > 0 && y < GRID_WIDTH - 1;
}

static void move_to(terrain_map_t *m, struct character_s *c, int nx, int ny)
{
  m->cmap[c->location.x][c->location.y] = NULL;
  c->travel_cost += c->cost(*char_to_tile_s(m->grid[nx][ny]));
  c->location.x = nx;
  c->location.y = ny;
  m->cmap[nx][ny] = c;
}

static bool can_enter(terrain_map_t *m, struct character_s *c, int nx, int ny)
{
  if (!in_bounds(nx, ny)) return false;
  if (m->cmap[nx][ny]) return false;
  return c->cost(*char_to_tile_s(m->grid[nx][ny])) != INT32_MAX;
}

/* Greedy descent on a distance map toward the PC. */
static void follow_dist(terrain_map_t *m, struct character_s *c,
                        int32_t dist[GRID_HEIGHT][GRID_WIDTH])
{
  int bestdx = 0, bestdy = 0;
  int32_t best = dist[c->location.x][c->location.y];

  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      if (dx == 0 && dy == 0) continue;
      int nx = c->location.x + dx, ny = c->location.y + dy;
      if (!in_bounds(nx, ny) || m->cmap[nx][ny]) continue;
      if (dist[nx][ny] < best) {
        best = dist[nx][ny];
        bestdx = dx;
        bestdy = dy;
      }
    }
  }
  if (bestdx || bestdy) {
    move_to(m, c, c->location.x + bestdx, c->location.y + bestdy);
  }
}

static void move_hiker(terrain_map_t *m, struct character_s *c)
{
  follow_dist(m, c, hiker_dist);
}

static void move_rival(terrain_map_t *m, struct character_s *c)
{
  follow_dist(m, c, rival_dist);
}

static void move_pacer(terrain_map_t *m, struct character_s *c)
{
  int dx = c->cardinal ? c->direction : 0;
  int dy = c->cardinal ? 0 : c->direction;

  if (can_enter(m, c, c->location.x + dx, c->location.y + dy)) {
    move_to(m, c, c->location.x + dx, c->location.y + dy);
  } else {
    c->direction = -c->direction;   /* bounce off the obstacle */
  }
}

/* Wanderer only steps onto the same terrain type it is standing on. */
static void move_wanderer(terrain_map_t *m, struct character_s *c)
{
  int dx = rand() % 3 - 1, dy = rand() % 3 - 1;
  if (dx == 0 && dy == 0) return;

  int nx = c->location.x + dx, ny = c->location.y + dy;
  if (!can_enter(m, c, nx, ny)) return;
  if (m->grid[nx][ny] != m->grid[c->location.x][c->location.y]) return;
  move_to(m, c, nx, ny);
}

static void move_sentry(terrain_map_t *m, struct character_s *c)
{
  (void) m;
  (void) c;
  /* Sentries stay put. */
}

static void move_explorer(terrain_map_t *m, struct character_s *c)
{
  int dx = rand() % 3 - 1, dy = rand() % 3 - 1;
  if (dx == 0 && dy == 0) return;

  int nx = c->location.x + dx, ny = c->location.y + dy;
  if (can_enter(m, c, nx, ny)) {
    move_to(m, c, nx, ny);
  }
}

/* Swimmers only step onto water. */
static void move_swimmer(terrain_map_t *m, struct character_s *c)
{
  int dx = rand() % 3 - 1, dy = rand() % 3 - 1;
  if (dx == 0 && dy == 0) return;

  int nx = c->location.x + dx, ny = c->location.y + dy;
  if (!in_bounds(nx, ny) || m->cmap[nx][ny]) return;
  if (m->grid[nx][ny] == waterTile.ascii) {
    move_to(m, c, nx, ny);
  }
}

typedef void (*move_fn)(terrain_map_t *, struct character_s *);

static move_fn move_table[NUM_MOVEMENT_TYPES] = {
  [MOVE_HIKER]    = move_hiker,
  [MOVE_RIVAL]    = move_rival,
  [MOVE_PACER]    = move_pacer,
  [MOVE_WANDERER] = move_wanderer,
  [MOVE_SENTRY]   = move_sentry,
  [MOVE_EXPLORER] = move_explorer,
  [MOVE_SWIMMER]  = move_swimmer,
  [MOVE_PC]       = NULL,
};

void move_npcs(terrain_map_t *map)
{
  for (int i = 0; i < NUM_NPCS; i++) {
    struct character_s *c = npc_list[i];
    if (c->mtype < NUM_MOVEMENT_TYPES && move_table[c->mtype]) {
      move_table[c->mtype](map, c);
    }
  }
}