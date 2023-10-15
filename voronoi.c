#include <math.h>
#include "voronoi.h"
#include "tiles.h"
#include "characters.h"

#define distance(x1,y1,x2,y2) sqrt ((x - x2) * (x - x2) + (y -y2) *(y -y2))

 struct tile_s pokemonCenter = 
{
    .name = "center",
    .ascii = 'C',
    .tile_idx = PokeCenter
};

struct tile_s pokeMart = 
{
    .name = "mart",
    .ascii = 'M',
    .tile_idx = PokeMart,
};


void generate_voronoi_map(terrain_map_t *map, Gates_t gates)
{
    Seed_t seeds[SEED_NUM] = {0};
    generate_map_border(map);
    generate_voronoi_seeds(&seeds[0]);
    fill_vorinoi_map(map, &seeds[0]);
    generate_map_gates(map, &gates);
    map->gates = gates;
    generate_map_path(map, gates);
    place_center(map);
    place_mart(map);
}

void generate_map_border(terrain_map_t* map)
{
    for(int i = 0; i < GRID_HEIGHT; i++) {
        map->grid[i][0] = &mountainTile;
        map->grid[i][GRID_WIDTH - 1] =  &mountainTile;
    }

    for(int j = 0; j < GRID_WIDTH; j++) {
        map->grid[0][j] = &mountainTile;
        map->grid[GRID_HEIGHT - 1][j] = &mountainTile;
    }

    for(int i = 1; i < GRID_HEIGHT - 1; i++) {
        for (int j = 1; j < GRID_WIDTH - 1; j++) {
            map->grid[i][j] = &clearingTile;
        }
    }
}

void generate_voronoi_seeds(Seed_t* seeds)
{
    for(int i = 0; i < SEED_NUM; i++) {
        seeds[i].loc.x = 1 + (rand() % (GRID_HEIGHT - 1));
        seeds[i].loc.y = 1 + (rand() % (GRID_WIDTH - 1));
        seeds[i].tile = tiles[i % (SpawnTileCount)];
    }
    // TODO: choose seed at random instead of this distribution
}

void fill_vorinoi_map(terrain_map_t *map, Seed_t* seeds)
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
            if(seeds[closestSeed].tile == &treeTile) {
                int treeChance = rand() % 100;
                if(treeChance < 20) {
                    map->grid[x][y] = &treeTile;
                } 
                else {
                    map->grid[x][y] = &clearingTile;
                }
            }
            else {    
                map->grid[x][y] = seeds[closestSeed].tile;
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
    map->grid[gates->left.x][gates->left.y]     = &pathTile;
    map->grid[gates->right.x][gates->right.y]   = &pathTile;
    map->grid[gates->bottom.x][gates->bottom.y] = &pathTile;
    map->grid[gates->top.x][gates->top.y]       = &pathTile;
}

void generate_map_path(terrain_map_t *m, Gates_t gates)
{
    int direction;

    
    // left gate: go right until bottom gate
    while(gates.left.y < gates.bottom.y) {
        m->grid[gates.left.x][gates.left.y] = &pathTile;
        gates.left.y++;
    }


    direction = gates.left.x > gates.right.x ? -1 : 1;
    // left gate: go in virtical direction of right gate
    while(gates.left.x != gates.right.x) {
        m->grid[gates.left.x][gates.left.y] = &pathTile;
        gates.left.x += direction;
    }
    

    // left gate: go rest of way in horizontal to right gate
    while(gates.left.y != GRID_WIDTH) {
        m->grid[gates.left.x][gates.left.y] = &pathTile;
        gates.left.y++;
    }


    // bottom gate: go up until right gate (gates.left.x has been changed)
    while(gates.bottom.x > gates.left.x) {
        m->grid[gates.bottom.x][gates.bottom.y] = &pathTile;
        gates.bottom.x--;
    }

    
    // bottom gate: go in horizontal direction of top gate
    direction = gates.bottom.y > gates.top.y ? -1 : 1;
    while(gates.bottom.y != gates.top.y) {
        m->grid[gates.bottom.x][gates.bottom.y] = &pathTile;
        gates.bottom.y += direction;
    }

    // bottom gate: go rest of the way up to top gate
    while(gates.bottom.x != 0) {
        m->grid[gates.bottom.x][gates.bottom.y] = &pathTile;
        gates.bottom.x--;
    }

    if(m->location.y == 400){
        m->grid[gates.top.x][gates.top.y] = &mountainTile;
    }
    if(m->location.x == 0){
        m->grid[gates.left.x][gates.left.y] = &mountainTile;
    }
    if(m->location.y == 0){
        m->grid[gates.bottom.x][gates.bottom.y] = &mountainTile;
    }
    if(m->location.x == 400){
        m->grid[gates.right.x][gates.right.y] = &mountainTile;
    }

}


 void find_building_location(terrain_map_t *m, Location_t *l)
{
    do{
      l->x = rand() % (GRID_HEIGHT -3) +1;
      l->y = rand() % (GRID_WIDTH -3) +1;

        if((m->grid[l->x-1][l->y]    == &pathTile    &&
          m->grid[l->x-1][l->y+1]    == &pathTile)   ||
          (m->grid[l->x+2][l->y]     == &pathTile    &&
          m->grid[l->x+2][l->y+1]    == &pathTile)   ||
          (m->grid[l->x][l->y-1]     == &pathTile    &&
          m->grid[l->x+1][l->y-1]    == &pathTile)   ||
          (m->grid[l->x][l->y+2]     == &pathTile    &&
          m->grid[l->x+1][l->y+2]    == &pathTile    &&
          m->grid[l->x][l->y]        != &pokeMart    &&
          m->grid[l->x][l->y]        != &pokemonCenter  &&
          m->grid[l->x+1][l->y]      != &pokeMart    &&
          m->grid[l->x+1][l->y]      != &pokemonCenter  &&
          m->grid[l->x][l->y+1]      != &pokeMart    &&
          m->grid[l->x][l->y+1]      != &pokemonCenter  &&
          m->grid[l->x+1][l->y+1]    != &pokeMart    &&
          m->grid[l->x+1][l->y+1]    != &pokemonCenter  &&
          m->grid[l->x][l->y]        != &pathTile    &&
          m->grid[l->x+1][l->y]      != &pathTile    &&
          m->grid[l->x][l->y+1]      != &pathTile    &&
          m->grid[l->x+1][l->y+1]    != &pathTile)) {
            break;
          }
    }while(1); 
}



void place_mart(terrain_map_t *m)
{   
   int manhattan_dist;
    int building_chance;
    int chance_center;

    if(m->location.x == 200 && m->location.y == 200) {
        Location_t l;
        find_building_location(m, &l);

        m->grid[l.x][l.y]      = &pokeMart;
        m->grid[l.x +1][l.y]   = &pokeMart;
        m->grid[l.x][l.y+1]    = &pokeMart;
        m->grid[l.x +1][l.y+1] = &pokeMart;
    }
    else {
        // TODO: make mangattan distance use offset from 200, 200 (since that is 0,0)
        manhattan_dist = abs(0 - m->location.x) + abs(0 - m->location.y);
        building_chance = (((manhattan_dist * -45)/200)+40)/-1;
        chance_center = rand() % 100;
        if(chance_center > building_chance){
            Location_t l;
            find_building_location(m, &l);

            m->grid[l.x][l.y]      = &pokeMart;
            m->grid[l.x +1][l.y]   = &pokeMart;
            m->grid[l.x][l.y+1]    = &pokeMart;
            m->grid[l.x +1][l.y+1] = &pokeMart; 
        }
    }
}


void place_center(terrain_map_t *m)
{   
    int manhattan_dist;
    int building_chance;
    int chance_center;

    if(m->location.x == 200 && m->location.y == 200){
        Location_t l;
        find_building_location(m, &l);

        m->grid[l.x][l.y]      = &pokemonCenter;
        m->grid[l.x +1][l.y]   = &pokemonCenter;
        m->grid[l.x][l.y+1]    = &pokemonCenter;
        m->grid[l.x +1][l.y+1] = &pokemonCenter;
    } else {
        // TODO: copy other manhattan fix
        manhattan_dist = abs(0- m->location.x) + abs(0 - m->location.y);
        building_chance = (((manhattan_dist * -45)/200)+40)/-1;
        chance_center = rand() % 100;
        if(chance_center > building_chance){
        Location_t l;
        find_building_location(m, &l);

        m->grid[l.x][l.y]      = &pokemonCenter;
        m->grid[l.x +1][l.y]   = &pokemonCenter;
        m->grid[l.x][l.y+1]    = &pokemonCenter;
        m->grid[l.x +1][l.y+1] = &pokemonCenter; 
        }
    }
}








