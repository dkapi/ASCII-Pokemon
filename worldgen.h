#ifndef _WORLDGEN_H_
#define _WORLDGEN_H_

#include <stdint.h>
#include <stdlib.h>

#define GRID_HEIGHT 24 - 3
#define GRID_WIDTH 80
#define WORLD_HEIGHT 401
#define WORLD_WIDTH 401
#define WORLD_SIZE 401
#define SEED_NUM 20

#define BLACK   "\e[0;30m"
#define RED     "\e[0;31m"
#define GREEN   "\e[0;32m"
#define YELLOW  "\e[38;5;137m"
#define BLUE    "\e[0;34m"
#define MAGENTA "\e[0;35m"
#define CYAN    "\e[0;36m"
#define WHITE   "\e[0;37m"
#define BROWN   "\e[38;5;94m"
#define LBLUE   "\e[38;5;33m"

typedef struct Location 
{
    int x;
    int y;
} Location_t;

typedef struct Gates
{
    Location_t left;
    Location_t right;
    Location_t bottom;
    Location_t top;
} Gates_t;

typedef struct terrain_map 
{
    char grid[GRID_HEIGHT][GRID_WIDTH];
    Location_t location;
    Gates_t gates;
    int visited, cost, distance;
} terrain_map_t;


#endif /* _WORLDGEN_H_ */