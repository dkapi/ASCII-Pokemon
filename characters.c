#include "characters.h"


// .cost is a function pointer that we are assign for each character type
struct character_s pc = 
{
  .ascii = '@',
  .cost = pc_cost,
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

uint32_t pc_cost(struct tile_s tile) 
{
	switch(tile.tile_idx) {
		case Path:
			return 10;
		case TreeTile:
			return INT32_MAX;
		default:
			// should not reach here
			return INT32_MAX;
  	}

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