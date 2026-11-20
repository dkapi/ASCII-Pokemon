#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "worldgen.h"
#include "tiles.h"

typedef enum movement_type {
  MOVE_HIKER,
  MOVE_RIVAL,
  MOVE_PACER,
  MOVE_WANDERER,
  MOVE_SENTRY,
  MOVE_EXPLORER,
  MOVE_SWIMMER,
  MOVE_PC,
  NUM_MOVEMENT_TYPES
} movement_type_t;

typedef enum character_type {
  CHAR_PC,
  CHAR_HIKER,
  CHAR_RIVAL,
  CHAR_SWIMMER,
  CHAR_OTHER,
  NUM_CHAR_TYPES
} character_type_t;

struct character_s {
  char ascii;
  Location_t location;
  int direction;      /* facing sign for pacers */
  int cardinal;       /* pacer axis: 0 = vertical, 1 = horizontal */
  movement_type_t mtype;
  character_type_t ctype;
  int defeated;
  uint32_t travel_cost;
  uint32_t (*cost)(struct tile_s);
};

extern struct character_s pc;
extern struct character_s hiker;
extern struct character_s rival;
extern struct character_s pacer;
extern struct character_s sentry;
extern struct character_s wanderer;
extern struct character_s explorer;
extern struct character_s swimmer;

/* Distance-to-PC (movement cost) maps used by the hiker and rival NPCs. */
extern int32_t hiker_dist[GRID_HEIGHT][GRID_WIDTH];
extern int32_t rival_dist[GRID_HEIGHT][GRID_WIDTH];

uint32_t pc_cost(struct tile_s tile);
uint32_t hiker_cost(struct tile_s tile);
uint32_t rival_cost(struct tile_s tile);
uint32_t swimmer_cost(struct tile_s tile);
uint32_t other_cost(struct tile_s tile);

void clear_characters(terrain_map_t *map);
void place_pc(terrain_map_t *map);
void place_all_npcs(terrain_map_t *map);
void move_npcs(terrain_map_t *map);

#endif /* CHARACTERS_H */