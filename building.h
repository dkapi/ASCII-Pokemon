#ifndef BUILDING_H
#define BUILDING_H

#include "worldgen.h"

#define BUILDING_CENTER 0
#define BUILDING_MART   1

/* Building-interior glyphs (reuse overworld tiles: '%' wall, '.' floor). */
#define BUILDING_DOOR  '#'   /* step on it to leave */
#define BUILDING_NURSE 'N'   /* Center: step on to heal */
#define BUILDING_PC    'P'   /* Center: step on to use the PC box */
#define BUILDING_CLERK 'S'   /* Mart: step on to shop */

/* Fill map->grid with a fixed building interior (a room with a counter NPC
 * at the top and a door at the bottom). */
void generate_building_interior(terrain_map_t *map, int type);

#endif /* BUILDING_H */