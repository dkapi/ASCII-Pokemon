/*
   Tile Definitions for wave fuction collapse algorithm
*/

#ifndef _TILES_H
#define _TILES_H

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

// TODO: optimize size?
enum tile_e 
{
   TreeTile,
   MountainTile,
   TallGrass,
   Clearing,
   Path,
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

struct tile_s tallGrass = 
{
   .name = "tall-grass",
   .ascii = ':',
   .tile_idx = TallGrass,
   .adjacency = {0.f},
};

struct tile_s clearing = 
{
   .name = "clearing",
   .ascii = '.',
   .tile_idx = Clearing,
   .adjacency = {0.f},
};

struct tile_s path =
{
    .name = "path",
    .ascii = '#',
    .tile_idx = Path,
    .adjacency = {0.f},
};

// important to keep in the order you defined in the enum
struct tile_s* tiles[TileCount] =
{
   &treeTile,
   &mountainTile,
   &tallGrass,
   &clearing,
   &path,
   
};


#endif /* _TILES_H */
