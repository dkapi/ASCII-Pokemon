#include "characters.h"


struct player pc = 
{
    .ascii = '@',
};

void find_loco_to_place(terrain_map_t *map, Location_t *l)
{
  (void)tiles[TileCount];
  do {
      l->x = rand() % (GRID_HEIGHT -3);
      l->y = rand() % (GRID_WIDTH -3);

      if(map->grid[l->x][l->y] == pathTile.ascii) {
        break;
      }
    } while(1); 
}



Location_t  place_character(terrain_map_t *map)
{
  (void)tiles[TileCount];
    Location_t l;
    find_loco_to_place(map, &l);

    map->grid[l.x][l.y] = pc.ascii;
    pc.location.x = l.x;
    pc.location.y = l.y;
    pc.tile = &pathTile;
    
    return l;
}