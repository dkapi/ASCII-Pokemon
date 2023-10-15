/*
   Tile Definitions
*/
#ifndef _TILES_H
#define _TILES_H


// TODO: optimize size?
// TODO: fix these names
enum tile_e 
{
   TreeTile,
   MountainTile,
   TallGrassTile,
   ClearingTile,
   PathTile,
   WaterTile,
   PokeMart,
   PokeCenter,
   EdgeTile,
   TileCount,
   SpawnTileCount = TileCount - 4,
};

struct tile_s
{
   const char* name;
   char ascii;
   enum tile_e tile_idx;
};

static struct tile_s treeTile = 
{
   .name = "tree",
   .ascii = '^',
   .tile_idx = TreeTile,
};

static struct tile_s mountainTile = 
{
   .name = "mountain",
   .ascii = '%',
   .tile_idx = MountainTile,
};

static struct tile_s tallGrassTile = 
{
   .name = "tall-grass",
   .ascii = ':',
   .tile_idx = TallGrassTile,
};

static struct tile_s clearingTile = 
{
   .name = "clearing",
   .ascii = '.',
   .tile_idx = ClearingTile,
};

static struct tile_s pathTile =
{
   .name = "path",
   .ascii = '#',
   .tile_idx = PathTile,
};

static struct tile_s waterTile =
{
   .name = "water",
   .ascii = '~',
   .tile_idx = WaterTile,
};

extern struct tile_s pokemonCenter;
extern struct tile_s pokeMart;
extern struct tile_s edgeTile;

static struct tile_s* tiles[SpawnTileCount] =
{
   &treeTile,
   &mountainTile,
   &tallGrassTile,
   &clearingTile,
   &waterTile,   
};

#endif /* _TILES_H */
