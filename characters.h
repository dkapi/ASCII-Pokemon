#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "worldgen.h"
#include "tiles.h"

struct player {
    struct tile_s *tile;
    const char ascii;
    Location_t location;
};

extern struct player pc;


void find_loco_to_place(terrain_map_t *map, Location_t *l);
Location_t place_character(terrain_map_t *map);

#endif /* CHARACTERS_H*/