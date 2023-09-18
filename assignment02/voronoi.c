#include <math.h>
#include "voronoi.h"
#include "tiles.h"

static inline double distance(int x, int y, int x2, int y2)
{
    return sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
}

void generate_voronoi_map(terrain_map_t *map, Gates_t gates)
{
    Seed_t seeds[SEED_NUM] = {0};
    generate_map_border(map);
    generate_voronoi_seeds(&seeds[0]);
    fill_vorinoi_map(map, &seeds[0]);
    generate_map_gates(map, &gates);
    map->gates = gates;
    generate_map_path(map, gates);
}

void generate_map_border(terrain_map_t* gridMap)
{
    for(int i = 0; i < GRID_HEIGHT; i++) {
        gridMap->grid[i][0] = mountainTile.ascii;
        gridMap->grid[i][GRID_WIDTH - 1] = mountainTile.ascii;
    }

    for(int j = 0; j < GRID_WIDTH; j++) {
        gridMap->grid[0][j] = mountainTile.ascii;
        gridMap->grid[GRID_HEIGHT - 1][j] = mountainTile.ascii;
    }

    for(int i = 1; i < GRID_HEIGHT - 1; i++) {
        for (int j = 1; j < GRID_WIDTH - 1; j++) {
            gridMap->grid[i][j] = clearingTile.ascii;
        }
    }
}

void generate_voronoi_seeds(Seed_t* seeds)
{
    for(int i = 0; i < SEED_NUM; i++) {
        seeds[i].loc.x = 1 + (rand() % (GRID_HEIGHT - 1));
        seeds[i].loc.y = 1 + (rand() % (GRID_WIDTH - 1));
        seeds[i].tile = tiles[i % TileCount]->ascii;
    }
    // TODO: choose seed at random instead of this distribution
}

void fill_vorinoi_map(terrain_map_t *gridMap, Seed_t* seeds)
{
    // start at one because of border
    for(int x = 1; x < GRID_HEIGHT - 1; x++) {
        for(int y = 1; y < GRID_WIDTH - 1; y++) {
            int closestSeed = 0;
            double closestDist = distance(x, y, seeds[0].loc.x, seeds[0].loc.y);
            for(int i = 1; i < SEED_NUM; i++) {
                double currDist = distance(x, y, seeds[i].loc.x, seeds[i].loc.y);
                if(currDist < closestDist) {
                    closestSeed = i;
                    closestDist = currDist;
                }
            }
            // TODO: this is not desired way to do this but it works
            // prolly write a switch statement instead
            if(seeds[closestSeed].tile == treeTile.ascii) {
                int treeChance = rand() % 100;
                if(treeChance < 25) {
                    gridMap->grid[x][y] = treeTile.ascii;
                } 
                else {
                    gridMap->grid[x][y] = clearingTile.ascii;
                }
            }
            else {
                gridMap->grid[x][y] = seeds[closestSeed].tile;
            }
        }
    }
}

void generate_map_gates(terrain_map_t *map, Gates_t* gates) 
{
    // these may seem "backwards" but x gives us row and y gives entry in that row
    if(gates->left.x == 0 && gates->left.y == 0) {
        gates->left.x = 1 + (rand() % (GRID_HEIGHT - 2));
        gates->left.y = 0; 
    }
    if(gates->right.x == 0 && gates->right.y == 0) {
        gates->right.x = 1 + (rand() % (GRID_HEIGHT - 2));
        gates->right.y = GRID_WIDTH - 1; 
    }
    if(gates->bottom.x == 0 && gates->bottom.y == 0) {
        gates->bottom.x = GRID_HEIGHT - 1;
        gates->bottom.y = 1 + (rand() % (GRID_WIDTH - 2));
    }
    if(gates->top.x == 0 && gates->top.y == 0) {
        gates->top.x = 0;
        gates->top.y = 1 + (rand() % (GRID_WIDTH - 2));
    }
    map->grid[gates->left.x][gates->left.y]     = pathTile.ascii;
    map->grid[gates->right.x][gates->right.y]   = pathTile.ascii;
    map->grid[gates->bottom.x][gates->bottom.y] = pathTile.ascii;
    map->grid[gates->top.x][gates->top.y]       = pathTile.ascii;
}
void generate_map_path(terrain_map_t *m, Gates_t gates)
{
    int direction;

    
    // left gate: go right until bottom gate
    while(gates.left.y < gates.bottom.y) {
        m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
        gates.left.y++;
    }


    direction = gates.left.x > gates.right.x ? -1 : 1;
    // left gate: go in virtical direction of right gate
    while(gates.left.x != gates.right.x) {
        m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
        gates.left.x += direction;
    }
    

    // left gate: go rest of way in horizontal to right gate
    while(gates.left.y != GRID_WIDTH) {
        m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
        gates.left.y++;
    }


    // bottom gate: go up until right gate (gates.left.x has been changed)
    while(gates.bottom.x > gates.left.x) {
        m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
        gates.bottom.x--;
    }

    
    // bottom gate: go in horizontal direction of top gate
    direction = gates.bottom.y > gates.top.y ? -1 : 1;
    while(gates.bottom.y != gates.top.y) {
        m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
        gates.bottom.y += direction;
    }

    // bottom gate: go rest of the way up to top gate
    while(gates.bottom.x != 0) {
        m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
        gates.bottom.x--;
    }
}
 void find_building_location(terrain_map_t *m)
{
    int x, y;
    do{
        x = rand() % (GRID_HEIGHT -3) +1;
        y = rand() % (GRID_WIDTH -3) +1;

        if((m->grid[x][y]   == pathTile.ascii    &&
          m->grid[x-1][y+1] == pathTile.ascii)    ||
          (m->grid[x+2][y]     == pathTile.ascii    &&
          m->grid[x+2][y+1] == pathTile.ascii)    ||
          (m->grid[x][y-1] == pathTile.ascii    &&
          m->grid[x+1][y-1] == pathTile.ascii)    ||
          (m->grid[x][y+2] == pathTile.ascii    &&
          m->grid[x+1][y+2] == pathTile.ascii    &&
          m->grid[x][y]     != pokeMart.ascii    &&
          m->grid[x][y]     != pokemonCenter.ascii  &&
          m->grid[x+1][y]     != pokeMart.ascii    &&
          m->grid[x+1][y]     != pokemonCenter.ascii  &&
          m->grid[x][y+1] != pokeMart.ascii    &&
          m->grid[x][y+1] != pokemonCenter.ascii  &&
          m->grid[x+1][y+1] != pokeMart.ascii    &&
          m->grid[x+1][y+1] != pokemonCenter.ascii  &&
          m->grid[x][y]     != pathTile.ascii    &&
          m->grid[x+1][y]     != pathTile.ascii    &&
          m->grid[x][y+1] != pathTile.ascii    &&
          m->grid[x+1][y+1] != pathTile.ascii)) {
            break;
          }
          
    }while(1); 

}






