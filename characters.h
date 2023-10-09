#ifndef CHARACTERS_H
#define CHARACTERS_H
#include <stdio.h>
#include "worldgen.h"
#include "tiles.h"
#include "heap.h"

typedef struct dijk_map_t
{
     Location_t pos;
    uint32_t cost;
    heap_node_t *hn;
     struct tile_s *tile;
    struct character_s *npc;
}dijk_map_t;

struct character_s {
    int direction;
    const char ascii;
    Location_t location;
    struct tile_s *tile;
    uint32_t travelCost;
    uint32_t (*cost)(struct tile_s);  // func pointer accessed ex pc.cost(tile)
    dijk_map_t costMap[GRID_HEIGHT][GRID_WIDTH];
};


extern struct character_s pc;
extern struct character_s hiker;
extern struct character_s rival;
extern struct character_s pacer;
extern struct character_s sentry;
extern struct character_s swimmer;
extern struct character_s wanderer;
extern struct character_s explorer;


void find_loco_to_place_pc(terrain_map_t *map, Location_t *l);
void find_loco_to_place_npc(terrain_map_t *map, Location_t *l, struct character_s npc);

uint32_t pc_cost(struct tile_s tile);
uint32_t hiker_cost(struct tile_s tile );
uint32_t rival_cost(struct tile_s tile );
uint32_t other_cost(struct tile_s tile );
uint32_t swimmer_cost(struct tile_s tile);

void place_pc(terrain_map_t *map);
void place_npc(terrain_map_t *map, struct character_s *npc);

struct tile_s* char_to_tile_s(char terrain);


#endif /* CHARACTERS_H*/