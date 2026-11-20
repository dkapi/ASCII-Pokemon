#include "characters.h"

/* .cost is a function pointer assigned per character type. */
struct character_s pc = {
  .ascii = '@',
  .cost = pc_cost,
};

void find_loco_to_place(terrain_map_t *map, Location_t *l)
{
  do {
    l->x = rand() % (GRID_HEIGHT - 3);
    l->y = rand() % (GRID_WIDTH - 3);

    if (map->grid[l->x][l->y] == pathTile.ascii) {
      break;
    }
  } while (1);
}

uint32_t pc_cost(struct tile_s tile)
{
  switch (tile.tile_idx) {
    case Path:
    case PokeCenter:
    case PokeMart:
    case Clearing:
      return 10;
    case TallGrass:
      return 20;
    case TreeTile:
    case MountainTile:
    case Water:
      return INT32_MAX;
    default:
      /* should not reach here */
      return INT32_MAX;
  }
}

uint32_t hiker_cost(struct tile_s tile)
{
  switch (tile.tile_idx) {
    case Path:       return 10;
    case Clearing:   return 10;
    case PokeCenter: return 50;
    case PokeMart:   return 50;
    case MountainTile: return 15;
    case TreeTile:     return 15;
    case TallGrass:    return 15;
    case Water:        return INT32_MAX;
    default:
      /* should not reach here */
      return INT32_MAX;
  }
}

uint32_t rival_cost(struct tile_s tile)
{
  switch (tile.tile_idx) {
    case Path:       return 10;
    case Clearing:   return 10;
    case PokeCenter: return 50;
    case PokeMart:   return 50;
    case TallGrass:  return 20;
    case MountainTile: return INT32_MAX;
    case TreeTile:     return INT32_MAX;
    case Water:        return INT32_MAX;
    default:
      /* should not reach here */
      return INT32_MAX;
  }
}

Location_t place_character(terrain_map_t *map)
{
  Location_t l;
  find_loco_to_place(map, &l);

  /* Do NOT stamp '@' into the terrain grid; print_map() draws the PC on
   * top at this location instead. */
  pc.location.x = l.x;
  pc.location.y = l.y;
  pc.tile = &pathTile;

  return l;
}