#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "worldgen.h"
#include "tiles.h"

struct character_s {
    struct tile_s *tile;
    const char ascii;
    Location_t location;
    uint32_t (*cost)(struct tile_s);  // func pointer accessed ex pc.cost(tile)
};

extern struct character_s pc;

void find_loco_to_place(terrain_map_t *map, Location_t *l);

uint32_t pc_cost(struct tile_s tile);
uint32_t hiker_cost(struct tile_s tile );
uint32_t rival_cost(struct tile_s tile );

Location_t place_character(terrain_map_t *map);

#endif /* CHARACTERS_H*/