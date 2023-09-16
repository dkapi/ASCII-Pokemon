/*
   Tile Definitions
*/

#ifndef _TILES_H
#define _TILES_H


// TODO: optimize size?
enum tile_e 
{
   TreeTile,
   MountainTile,
   TallGrass,
   Clearing,
   // Path,
   Water,
   TileCount,
};

struct tile_s
{
   const char* name;
   const char ascii;
   const enum tile_e tile_idx;  // TODO: is this needed?
   const float adjacency[TileCount];
};

struct tile_s treeTile = 
{
   .name = "tree",
   .ascii = '^',
   .tile_idx = TreeTile,
   .adjacency = {0.f},
};

struct tile_s mountainTile = 
{
   .name = "mountain",
   .ascii = '%',
   .tile_idx = MountainTile,
   .adjacency = {0.f},
};

struct tile_s tallGrassTile = 
{
   .name = "tall-grass",
   .ascii = ':',
   .tile_idx = TallGrass,
   .adjacency = {0.f},
};

struct tile_s clearingTile = 
{
   .name = "clearing",
   .ascii = '.',
   .tile_idx = Clearing,
   .adjacency = {0.f},
};

struct tile_s pathTile =
{
    .name = "path",
    .ascii = '#',
    // .tile_idx = Path,
    .adjacency = {0.f},
};

struct tile_s waterTile =
{
    .name = "water",
    .ascii = '~',
    .tile_idx = Water,
    .adjacency = {0.f},
};

struct tile_s pokemonCenter = 
{
 .name = "center",
 .ascii = 'C',
 //.tile_idx = Center,
 .adjacency = {0.f},
};

struct tile_s pokeMart = 
{
   .name = "mart",
   .ascii = 'M',
   .adjacency = {0.f},
};


// important to keep in the order you defined in the enum
struct tile_s* tiles[TileCount] =
{
   &treeTile,
   &mountainTile,
   &tallGrassTile,
   &clearingTile,
   // &path,
   &waterTile,
   
};


#endif /* _TILES_H */