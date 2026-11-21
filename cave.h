#ifndef CAVE_H
#define CAVE_H

#include "worldgen.h"
#include "tiles.h"

/* Cave glyphs.  The cave reuses overworld tiles so existing movement costs
 * stay correct: mountain '%' is impassable rock, clearing '.' is walkable
 * floor, and a tunnel gate is a path '#' (walkable) on the border -- exactly
 * like the overworld's gate system, so the sideways cave network shares the
 * same leave_map() plumbing. */
#define CAVE_WALL  mountainTile.ascii    /* '%' */
#define CAVE_FLOOR clearingTile.ascii    /* '.' */
#define CAVE_EXIT  '>'
#define CAVE_MONEY '$'
#define CAVE_ITEM  '!'

#define CAVE_VISION 7          /* torch light radius (chebyshev) in caves */
#define CAVE_ENCOUNTER_PROB 10 /* % chance per cave-floor step */

/* Generate an organic cave (cellular automata) into map->grid: a single
 * connected floor region, four tunnel gates linking the sideways cave network,
 * an exit stair, and a few loot pickups. */
void generate_cave_map(terrain_map_t *map, Gates_t gates);

#endif /* CAVE_H */