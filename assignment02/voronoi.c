#include <math.h>
// #include <unistd.h>  // TODO: why needed
#include "vorinoi.h"
#include "tiles.h"

static inline double distance(int x, int y, int x2, int y2)
{
    return sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
}

void generate_voronoi_map(terrain_map_t *gridMap)
{
    Seed_t seeds[SEED_NUM] = {0};
    generate_map_border(gridMap);
    generate_voronoi_seeds(&seeds[0]);
    fill_vorinoi_map(gridMap, &seeds[0]);
    Gates_t gates = {0}; // TODO: make sure this works with moving maps
    generate_map_gates(&gates);
    gridMap->grid[gates.left.x][gates.left.y] = pathTile.ascii;
    gridMap->grid[gates.right.x][gates.right.y] = pathTile.ascii;
    gridMap->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
    gridMap->grid[gates.top.x][gates.top.y] = pathTile.ascii;
    gridMap->gates = gates;
    generate_map_path(gridMap, gates);
}

void generate_map_border(terrain_map_t* gridMap)
{
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        gridMap->grid[i][0] = mountainTile.ascii;
        gridMap->grid[i][GRID_WIDTH - 1] = mountainTile.ascii;
    }

    for (int j = 0; j < GRID_WIDTH; j++)
    {
        gridMap->grid[0][j] = mountainTile.ascii;
        gridMap->grid[GRID_HEIGHT - 1][j] = mountainTile.ascii;
    }

    for (int i = 1; i < GRID_HEIGHT - 1; i++)
    {
        for (int j = 1; j < GRID_WIDTH - 1; j++)
        {
            gridMap->grid[i][j] = clearingTile.ascii;
        }
    }
}

void generate_voronoi_seeds(Seed_t* seeds)
{
    for (int i = 0; i < SEED_NUM; i++)
    {
        seeds[i].loc.x = 1 + (rand() % (GRID_HEIGHT - 1));
        seeds[i].loc.y = 1 + (rand() % (GRID_WIDTH - 1));
        seeds[i].tile = tiles[i % TileCount]->ascii;
    }
    // TODO: choose seed at random instead of this distribution
}

void fill_vorinoi_map(terrain_map_t *gridMap, Seed_t* seeds)
{
     // start at one because of border
    for (int x = 1; x < GRID_HEIGHT - 1; x++)
    {
        for (int y = 1; y < GRID_WIDTH - 1; y++)
        {
            int closestSeed = 0;
            double closestDist = distance(x, y, seeds[0].loc.x, seeds[0].loc.y);
            for (int i = 1; i < SEED_NUM; i++)
            {
                double currDist = distance(x, y, seeds[i].loc.x, seeds[i].loc.y);
                if (currDist < closestDist)
                {
                    closestSeed = i;
                    closestDist = currDist;
                }
            }
            // TODO: this is not desired way to do this but it works
            // prolly write a switch statement instead
            if (seeds[closestSeed].tile == treeTile.ascii)
            {
                int treeChance = rand() % 100;
                if (treeChance < 25)
                {
                    gridMap->grid[x][y] = treeTile.ascii;
                }
                else
                {
                    gridMap->grid[x][y] = clearingTile.ascii;
                }
            }
            else
            {
                gridMap->grid[x][y] = seeds[closestSeed].tile;
            }
        }
    }
}

void generate_map_gates(Gates_t* gates) 
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
}
void generate_map_path(terrain_map_t *m, Gates_t gates)
{
    int direction;

    // pick one of the 3 "runs" for placing a pokemart
    int PCplacement = 1 + rand() % 3;
    int PCx = 0, PCy = 0;
    int Mplacement = 1 + rand() % 3;
    int Mx = 0, My = 0;
    // place pokemon center on left run to bottom gate
    if (PCplacement == 1) {
        if(gates.left.x -1 != 0){
        PCx = gates.left.x - 1; // TODO: fixme
        } else if (gates.left.x +1 != 21){
            PCx = gates.left.x +1;
        }
        PCy = gates.left.y + (rand() % (gates.bottom.x - gates.left.y - 1));
       // printf("placement%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }
   
    
    // left gate: go right until bottom gate
    while (gates.left.y < gates.bottom.y)
    {
        m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
        gates.left.y++;
    }

    // place pokemon center on virtical run to right gate
    if (PCplacement == 2) {
        if((gates.left.x - gates.right.x) == 0) {
            PCx = gates.left.x;
        }
        else if( (gates.left.x-gates.right.x) < 0 ) {
            PCx = gates.left.x + (rand() % (gates.right.x-gates.left.x));
        } 
        else {
            PCx = gates.right.x + (rand() % (gates.left.x-gates.right.x));
        }
        PCy = gates.left.y - 1;
      //  printf("placement:%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }
   

    direction = gates.left.x > gates.right.x ? -1 : 1;
    // left gate: go in virtical direction of right gate
    while (gates.left.x != gates.right.x)
    {
        m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
        gates.left.x += direction;
    }
    
    // place pokemon center on last run to right gate
    if(PCplacement == 3) {
        PCx = gates.left.x - 1; // TODO: fixme
        PCy = gates.left.y + (rand() % (gates.right.y - gates.left.y + 1));
       // printf("placement:%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }

    // left gate: go rest of way in horizontal to right gate
    while (gates.left.y != GRID_WIDTH)
    {
        m->grid[gates.left.x][gates.left.y] = pathTile.ascii;
        gates.left.y++;
    }

    // pokemart first "run" bottom up to right gate height
    if(Mplacement == 1){
        // TODO: delete these dogshit if else if
        if(gates.bottom.y -1 != 0){
            My = gates.bottom.y - 1; // TODO: fixme
        } else if (gates.bottom.y +1 != GRID_WIDTH){
            My = gates.bottom.y +1;
        }
        Mx = gates.left.x + (rand() % (gates.bottom.x - gates.left.x + 1));
       // printf("placement%d, Mx:%d, My:%d\n", Mplacement, Mx, My); 
    }

    // bottom gate: go up until right gate (gates.left.x has been changed)
    while (gates.bottom.x > gates.left.x)
    {
        m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
        gates.bottom.x--;
    }

    // place mart on horizontal "run" of bot->top path
    if(Mplacement == 2) {
        if((gates.bottom.y - gates.top.y) == 0) {
            My = gates.bottom.y;
        }
        else if((gates.bottom.y-gates.top.y) < 0 ) {
            My = gates.bottom.y + (rand() % (gates.top.y-gates.bottom.y));
        } 
        else {
            My = gates.top.y + (rand() % (gates.bottom.y-gates.top.y));
        }
        Mx = gates.bottom.x + 1;
       // printf("placement:%d, Mx:%d, My:%d\n", Mplacement, Mx, My); 
    }

    // bottom gate: go in horizontal direction of top gate
    direction = gates.bottom.y > gates.top.y ? -1 : 1;
    while (gates.bottom.y != gates.top.y)
    {
        m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
        gates.bottom.y += direction;
    }
    Mplacement = 3;
    // place pokemon center on last run to top gate
    if(Mplacement == 3) {
        My = gates.bottom.y + 1; // TODO: fixme
        Mx = 1 + (rand() % (gates.bottom.x - gates.top.x + 1));
     //   printf("placement:%d, Mx:%d, My:%d\n", Mplacement, Mx, My); 
    }


    // bottom gate: go rest of the way up to top gate
    while (gates.bottom.x != 0)
    {
        m->grid[gates.bottom.x][gates.bottom.y] = pathTile.ascii;
        gates.bottom.x--;
    }

    m->grid[PCx][PCy] = pokemonCenter.ascii;
    m->grid[Mx][My] = pokeMart.ascii;
}
