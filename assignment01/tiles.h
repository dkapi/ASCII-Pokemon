/*
   Tile Definitions for wave fuction collapse algorithm
*/

#ifndef _TILES_H
#define _TILES_H

#define TILE_COUNT 10

struct tile_s
{
   const char* name;
   const char ascii;
   const float adjacency[TILE_COUNT];
};

static struct tile_s treeTile = 
{
   .name = "tree",
   .ascii = '^',
   .adjacency = {0.f},
};

static struct tile_s mountainTile = 
{
   .name = "mountain",
   .ascii = '%',
   .adjacency = {0.f},
};

static struct tile_s tallGrass = 
{
   .name = "tall-grass",
   .ascii = ':',
   .adjacency = {0.f},
};

static struct tile_s clearing = 
{
   .name = "clearing",
   .ascii = '.',
   .adjacency = {0.f},
};

#endif /* _TILES_H */
