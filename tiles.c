#include "tiles.h"

struct tile_s treeTile = {
  .name = "tree",
  .ascii = '^',
  .tile_idx = TreeTile,
};

struct tile_s mountainTile = {
  .name = "mountain",
  .ascii = '%',
  .tile_idx = MountainTile,
};

struct tile_s tallGrassTile = {
  .name = "tall-grass",
  .ascii = ':',
  .tile_idx = TallGrass,
};

struct tile_s clearingTile = {
  .name = "clearing",
  .ascii = '.',
  .tile_idx = Clearing,
};

struct tile_s pathTile = {
  .name = "path",
  .ascii = '#',
  .tile_idx = Path,
};

struct tile_s waterTile = {
  .name = "water",
  .ascii = '~',
  .tile_idx = Water,
};

struct tile_s pokemonCenter = {
  .name = "center",
  .ascii = 'C',
  .tile_idx = PokeCenter,
};

struct tile_s pokeMart = {
  .name = "mart",
  .ascii = 'M',
  .tile_idx = PokeMart,
};

/* Designated initializers keep this aligned with the enum regardless of
 * ordering.  (Previously Path was missing and Water occupied its slot.) */
const struct tile_s *tiles[TileCount] = {
  [TreeTile]     = &treeTile,
  [MountainTile] = &mountainTile,
  [TallGrass]    = &tallGrassTile,
  [Clearing]     = &clearingTile,
  [Path]         = &pathTile,
  [Water]        = &waterTile,
};