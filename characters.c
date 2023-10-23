#include "characters.h"
#include <stdio.h>
#include <stdbool.h>

struct character_s pc = 
{
    .self = { .ascii = '@', .name = "pcTile" },
    .travelCost =0,
    .cost = NULL,
};

struct character_s hiker = 
{
    .self = { .ascii = 'h', .name = "hikerTile" },
    .travelCost = 0,
    .cost = hiker_cost,
    .costMap = {},
};

struct character_s rival = 
{
    .self = { .ascii = 'r', .name = "rivalTile" },
    .travelCost = 0,
    .cost = rival_cost,
    .costMap = {},
};

struct character_s pacer =
{
    .direction = 1,
    .self = { .ascii = 'p', .name = "pacerTile" },
    .travelCost = 0,
    .cost = other_cost,
    .costMap = {},
};

struct character_s sentry =
{
    .self = { .ascii = 's', .name = "sentryTile" },
    .travelCost = 0,
    .cost = other_cost,
    .costMap = {},
};

struct character_s wanderer =
{
    .self = { .ascii = 'w', .name = "wandererTile" },
    .travelCost = 0,
    .cost = other_cost,
    .costMap = {},
};

struct character_s explorer =
{
    .self = { .ascii = 'e', .name = "explorerTile" },
    .travelCost = 0,
    .cost = other_cost,
    .costMap = {},
};

struct character_s swimmer =
{
    .self = { .ascii = 's', .name = "swimmerTile" },
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

        if(map->grid[l->x][l->y]->ascii == pathTile.ascii) {
        	break;
        }
    } while(1); 
}

static inline bool generic_spawn_check(terrain_map_t *map, Location_t* l) 
{
    return map->grid[l->x][l->y]->ascii == '@' || map->grid[l->x][l->y]->ascii == 'h' || 
        map->grid[l->x][l->y]->ascii == 'r' || map->grid[l->x][l->y]->ascii == '~' || 
        map->grid[l->x][l->y]->ascii == '%' || map->grid[l->x][l->y]->ascii == 'C' ||
        map->grid[l->x][l->y]->ascii == 'M' || map->grid[l->x][l->y]->ascii == '^' || 
        map->grid[l->x][l->y]->ascii == 'p' || map->grid[l->x][l->y]->ascii == 's' || 
        map->grid[l->x][l->y]->ascii == 'w' || map->grid[l->x][l->y]->ascii == 'e' ||
        map->grid[l->x][l->y]->ascii == '#';
}

static inline bool hiker_spawn_check(terrain_map_t *map, Location_t* l) 
{
    return map->grid[l->x][l->y]->ascii == '@' || map->grid[l->x][l->y]->ascii == 'h' || 
        map->grid[l->x][l->y]->ascii == 'r' || map->grid[l->x][l->y]->ascii == '~' || 
        map->grid[l->x][l->y]->ascii == 'C' || map->grid[l->x][l->y]->ascii == 'M' ||
        map->grid[l->x][l->y]->ascii == 'p' || map->grid[l->x][l->y]->ascii == 's' || 
        map->grid[l->x][l->y]->ascii == 'w' || map->grid[l->x][l->y]->ascii == 'e' ||
        map->grid[l->x][l->y]->ascii == '#';
}

/*
static inline bool swimmer_spawn_check(terrain_map_t *map, Location_t* l)
{
    return map->grid[l->x][l->y]->ascii == '~';
}
*/

void find_loco_to_place_npc(terrain_map_t *map, Location_t *l, struct character_s npc)
{
    do {
        l->x = 1+(rand() % (GRID_HEIGHT -3));
        l->y = 1+(rand() % (GRID_WIDTH - 3));
        switch(npc.self.ascii) {
            case('h'):
                if(hiker_spawn_check(map, l)) {
                    continue;
                }
                break;
            case('r'):
                if(generic_spawn_check(map, l)) {
                    continue;
                }
                break;
            case('p'):
                if(generic_spawn_check(map, l)) {
                    continue;
                }
                break;
            case('s'):
                if(generic_spawn_check(map, l)) {
                    continue;
                }
                break;
            case('w'):
                if(generic_spawn_check(map, l)) {
                    continue;
                }
                break;
            case('e'):
                if(generic_spawn_check(map, l)) {
                    continue;
                }
                break;
            default:
                fprintf(stderr, "unable to place this npc: %c", npc.standing->ascii);
                break;
        }
    break; // we matched a sucessful location break the random check
  } while(1);
}

void place_pc(terrain_map_t *map)
{
    (void)tiles[TileCount];
    Location_t l;
    find_loco_to_place_pc(map, &l);

    pc.standing = &pathTile;
    map->grid[l.x][l.y] = &pc.self;
    pc.location.x = l.x;
    pc.location.y = l.y;
}

void place_npc(terrain_map_t *map, struct character_s *npc)
{
    Location_t l;
    find_loco_to_place_npc(map, &l, *npc);
    
    npc->standing = map->grid[l.x][l.y];
    map->grid[l.x][l.y] = &npc->self;
    npc->location.x = l.x;
    npc->location.y = l.y;
}

uint32_t hiker_cost(struct tile_s tile)
{
    switch(tile.tile_idx) {
        case PathTile: 
            return 10;
        case ClearingTile:
            return 10;
        case PokeCenter:
            return 50;
        case PokeMart:
            return 50;
        case MountainTile:
            return 15;
        case TreeTile:
            return 15;
        case TallGrassTile:
            return 15;
        case WaterTile:
            return INT32_MAX;
        case EdgeTile:
            return INT32_MAX;
        default:
        // should not reach here
            return INT32_MAX;
    }
}

uint32_t rival_cost(struct tile_s tile)
{
  switch(tile.tile_idx) {
    case PathTile:
        return 10; 
    case ClearingTile:
        return 10;
    case PokeCenter:
        return 50;
    case PokeMart:
        return 50;
    case TallGrassTile:
        return 20;
    case MountainTile:
        return INT32_MAX;
    case TreeTile:
        return INT32_MAX;
    case WaterTile:
        return INT32_MAX;
    case EdgeTile:
        return INT32_MAX;
    default:
        // should not reach here
        return INT32_MAX;
  }    
}

uint32_t swimmer_cost(struct tile_s tile)
{
  switch(tile.tile_idx) {
    case PathTile:
        return INT32_MAX; 
    case ClearingTile:
        return INT32_MAX;
    case PokeCenter:
        return INT32_MAX;
    case PokeMart:
        return INT32_MAX;
    case TallGrassTile:
        return INT32_MAX;
    case MountainTile:
        return INT32_MAX;
    case TreeTile:
        return INT32_MAX;
    case WaterTile:
        return 7;
    case EdgeTile:
            return INT32_MAX;
    default:
      // should not reach here
      return INT32_MAX;
  }    
}

uint32_t other_cost(struct tile_s tile)
{
  switch(tile.tile_idx) {
    case PathTile:
        return 10; 
    case ClearingTile:
        return 10;
    case PokeCenter:
        return 50;
    case PokeMart:
        return 50;
    case TallGrassTile:
        return 20;
    case MountainTile:
        return INT32_MAX;
    case TreeTile:
        return INT32_MAX;
    case WaterTile:
        return INT32_MAX;
    case EdgeTile:
            return INT32_MAX;
    default:
      // should not reach here or tile is a npc
      return INT32_MAX;
    }    
}
