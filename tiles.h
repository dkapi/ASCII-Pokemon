/* Tile definitions.  The tile structs themselves live in tiles.c (one
 * definition each) so every translation unit shares the same instances. */
#ifndef _TILES_H
#define _TILES_H

enum tile_e {
  TreeTile,
  MountainTile,
  TallGrass,
  Clearing,
  Path,
  Water,
  TileCount,
  PokeMart,
  PokeCenter,
};

struct tile_s {
  const char *name;
  const char ascii;
  const enum tile_e tile_idx;
};

extern struct tile_s treeTile;
extern struct tile_s mountainTile;
extern struct tile_s tallGrassTile;
extern struct tile_s clearingTile;
extern struct tile_s pathTile;
extern struct tile_s waterTile;
extern struct tile_s pokemonCenter;
extern struct tile_s pokeMart;

/* Every map tile, indexed directly by tile_e (0 .. TileCount-1). */
extern const struct tile_s *tiles[TileCount];

#endif /* _TILES_H */