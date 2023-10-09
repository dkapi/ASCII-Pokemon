#include "characters.h"
#include <stdio.h>


// .cost is a function pointer that we assign for each character type
struct character_s pc = 
{
  .ascii = '@',
  .travelCost =0,
  .cost = pc_cost,
};

struct character_s hiker = 
{
  .ascii = 'h',
  .travelCost = 0,
  .cost = hiker_cost,
  .costMap = {},
};

struct character_s rival = 
{
  .ascii = 'r',
  .travelCost = 0,
  .cost = rival_cost,
  .costMap = {},
};

struct character_s pacer =
{
  .direction = 1,
  .ascii = 'p',
  .travelCost = 0,
  .cost = other_cost,
  .costMap = {},
};

struct character_s sentry =
{
  .ascii = 's',
  .travelCost = 0,
  .cost = other_cost,
  .costMap = {},
};

struct character_s wanderer =
{
  .ascii = 'w',
  .travelCost = 0,
  .cost = other_cost,
  .costMap = {},
};

struct character_s explorer =
{
  .ascii = 'e',
  .travelCost = 0,
  .cost = other_cost,
  .costMap = {},
};

struct character_s swimmer =
{
  .ascii = 's',
  .travelCost = 0,
  .cost = swimmer_cost,
  .costMap = {},
};


void find_loco_to_place_pc(terrain_map_t *map, Location_t *l)
{
    (void)tiles[TileCount];
    do {
        l->x = 1+(rand() % (GRID_HEIGHT -3));
        l->y = 1+(rand() % (GRID_WIDTH -3));

        if(map->grid[l->x][l->y] == pathTile.ascii) {
        	break;
        }
    } while(1); 
}

void find_loco_to_place_npc(terrain_map_t *map, Location_t *l, struct character_s npc)
{
  do {
      l->x = 1+(rand() % (GRID_HEIGHT -3));
      l->y = 1+(rand() % (GRID_WIDTH -3));
    switch(npc.ascii){
      case('h'):
        if(map->grid[l->x][l->y] == '@' || map->grid[l->x][l->y] == 'h' || map->grid[l->x][l->y] == 'r' ||
           map->grid[l->x][l->y] == '~' || map->grid[l->x][l->y] == 'C' || map->grid[l->x][l->y] == 'M' ){
          continue;
        }
        break;
      case('r'):
        if(map->grid[l->x][l->y] == '@' || map->grid[l->x][l->y] == 'h' || map->grid[l->x][l->y] == 'r' ||
           map->grid[l->x][l->y] == '~' || map->grid[l->x][l->y] == '%' || map->grid[l->x][l->y] == 'C' ||
           map->grid[l->x][l->y] == 'M' || map->grid[l->x][l->y] == '^' || map->grid[l->x][l->y] == '#' ) {
          continue;
           }
        break;
        case('p'):
        if(map->grid[l->x][l->y] == '@' || map->grid[l->x][l->y] == 'h' || map->grid[l->x][l->y] == 'r' ||
           map->grid[l->x][l->y] == '~' || map->grid[l->x][l->y] == '%' || map->grid[l->x][l->y] == 'C' ||
           map->grid[l->x][l->y] == 'M' || map->grid[l->x][l->y] == '^' || map->grid[l->x][l->y] == 'p' ||
           map->grid[l->x][l->y] == 's' || map->grid[l->x][l->y] == 'w' || map->grid[l->x][l->y] == 'e' ||
           map->grid[l->x][l->y] == '#') {
          continue;
           }
        break;
        case('s'):
        if(map->grid[l->x][l->y] == '@' || map->grid[l->x][l->y] == 'h' || map->grid[l->x][l->y] == 'r' ||
           map->grid[l->x][l->y] == '~' || map->grid[l->x][l->y] == '%' || map->grid[l->x][l->y] == 'C' ||
           map->grid[l->x][l->y] == 'M' || map->grid[l->x][l->y] == '^' || map->grid[l->x][l->y] == 'p' ||
           map->grid[l->x][l->y] == 's' || map->grid[l->x][l->y] == 'w' || map->grid[l->x][l->y] == 'e' ||
           map->grid[l->x][l->y] == '#') {
          continue;
           }
        break;
        case('w'):
        if(map->grid[l->x][l->y] == '@' || map->grid[l->x][l->y] == 'h' || map->grid[l->x][l->y] == 'r' ||
           map->grid[l->x][l->y] == '~' || map->grid[l->x][l->y] == '%' || map->grid[l->x][l->y] == 'C' ||
           map->grid[l->x][l->y] == 'M' || map->grid[l->x][l->y] == '^' || map->grid[l->x][l->y] == 'p' ||
           map->grid[l->x][l->y] == 's' || map->grid[l->x][l->y] == 'w' || map->grid[l->x][l->y] == 'e' ||
           map->grid[l->x][l->y] == '#') {
          continue;
           }
        break;
        case('e'):
        if(map->grid[l->x][l->y] == '@' || map->grid[l->x][l->y] == 'h' || map->grid[l->x][l->y] == 'r' ||
           map->grid[l->x][l->y] == '~' || map->grid[l->x][l->y] == '%' || map->grid[l->x][l->y] == 'C' ||
           map->grid[l->x][l->y] == 'M' || map->grid[l->x][l->y] == '^' || map->grid[l->x][l->y] == 'p' ||
           map->grid[l->x][l->y] == 's' || map->grid[l->x][l->y] == 'w' || map->grid[l->x][l->y] == 'e' ||
           map->grid[l->x][l->y] == '#') {
          continue;
           }
        break;
      default:
      fprintf(stderr, "unable to place this npc: %c", npc.ascii);
      break;
    }
    break;
  } while(1);
}

void place_pc(terrain_map_t *map)
{
  	(void)tiles[TileCount];
    Location_t l;
    find_loco_to_place_pc(map, &l);

    map->grid[l.x][l.y] = pc.ascii;
    pc.location.x = l.x;
    pc.location.y = l.y;
    pc.tile = &pathTile;
}

void place_npc(terrain_map_t *map, struct character_s *npc)
{
  Location_t l;
  find_loco_to_place_npc(map, &l, *npc);
 
  npc->tile = char_to_tile_s(map->grid[l.x][l.y]);
  npc->location.x = l.x;
  npc->location.y = l.y;
  map->grid[l.x][l.y] = npc->ascii;
}

uint32_t pc_cost(struct tile_s tile) 
{
	switch(tile.tile_idx) {
		case Path:
    case PokeCenter:
    case PokeMart:
    case Clearing:
			return 10;
		case TreeTile:
    case MountainTile:
    case Water:
			return INT32_MAX;
		default:
			// should not reach here
			return INT32_MAX;
  	}
}

uint32_t hiker_cost(struct tile_s tile)
{
  switch(tile.ascii) {
    case '#': 
      return 10;
    case '.':
      return 10;
    case 'C':
      return 50;
    case 'M':
      return 50;
    case '%':
      return 15;
    case '^':
      return 15;
    case ':':
      return 15;
    case '~':
      return INT32_MAX;
    default:
      // should not reach here
      return INT32_MAX;
  }
}

uint32_t rival_cost(struct tile_s tile)
{
  switch(tile.ascii){
    case '#':
      return 10; 
    case '.':
      return 10;
    case 'C':
      return 50;
    case 'M':
      return 50;
    case ':':
      return 20;
    case '%':
      return INT32_MAX;
    case '^':
      return INT32_MAX;
    case '~':
      return INT32_MAX;
    default:
      // should not reach here
      return INT32_MAX;
  }    
}

uint32_t swimmer_cost(struct tile_s tile)
{
  switch(tile.ascii){
    case '#':
      return INT32_MAX; 
    case '.':
      return INT32_MAX;
    case 'C':
      return INT32_MAX;
    case 'M':
      return INT32_MAX;
    case ':':
      return INT32_MAX;
    case '%':
      return INT32_MAX;
    case '^':
      return INT32_MAX;
    case '~':
      return 7;
    default:
      // should not reach here
      return INT32_MAX;
  }    
}

uint32_t other_cost(struct tile_s tile)
{
  switch(tile.ascii){
    case '#':
      return 10; 
    case '.':
      return 10;
    case 'C':
      return 50;
    case 'M':
      return 50;
      case ':':
      return 20;
    case '%':
      return INT32_MAX;
    case '^':
      return INT32_MAX;
    case '~':
      return INT32_MAX;
    default:
      // should not reach here
      // or tile is a npc
      return INT32_MAX;
  }    
}

struct tile_s* char_to_tile_s(char terrain){

    switch(terrain){
        case ':':
            return &tallGrassTile;
        case '^': 
            return &treeTile;
        case '~':
            return &waterTile;
        case '%':
            return &mountainTile;
        case '.':
            return &clearingTile;
        case 'C':
            return &pokemonCenter;
        case 'M':
            return &pokeMart;
        case '#':
            return &pathTile;
        case '@':
            return pc.tile;
        case 'h':
            return hiker.tile;
        case 'r':
            return rival.tile;
        case 'p':
            return pacer.tile;
        case 's':
            return sentry.tile;
        case 'w':
            return wanderer.tile;
        case 'e':
            return explorer.tile;
        default:
        // unreachable
        fprintf(stderr, "something went wrong in char_to_tile_s func: %c", terrain);
        return &pathTile;
        
    } 
}
