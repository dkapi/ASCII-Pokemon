#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "tiles.h"
#include <math.h>
#include <stdint.h>


#define GRID_HEIGHT 24 - 3
#define GRID_WIDTH 80
#define WORLD_HEIGHT 401
#define WORLD_WIDTH 401
#define SEED_NUM 20




typedef struct Seed
{
    unsigned int x;
    unsigned int y;
    char tile;
} Seed;

typedef struct map {
    char map[GRID_HEIGHT][GRID_WIDTH];
    int n, s, e, w;
    int visited;
} terrain_map_t;


//static terrain_map_t* worldGrid[401][401];
//static char gridMatrix[GRID_HEIGHT][GRID_WIDTH];
static int gates[4 * 2]; // (4 x,y pairs for gates)



void init_world(terrain_map_t *gridMap)
{
    
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        gridMap->map[i][0] = mountainTile.ascii;
        gridMap->map[i][GRID_WIDTH - 1] = mountainTile.ascii;
    }

    for (int j = 0; j < GRID_WIDTH; j++)
    {
        gridMap->map[0][j] = mountainTile.ascii;
        gridMap->map[GRID_HEIGHT - 1][j] = mountainTile.ascii;
    }

    for (int i = 1; i < GRID_HEIGHT - 1; i++)
    {
        for (int j = 1; j < GRID_WIDTH - 1; j++)
        {
            gridMap->map[i][j] = clearingTile.ascii;
        }
    }

    // these may seem "backwards" but x gives us row and y gives entry in that row
    gates[0] = 1 + (rand() % (GRID_HEIGHT - 2));
    gates[1] = 0; // left gate
    gridMap->w =gates[0];
    gates[2] = GRID_HEIGHT - 1;
    gates[3] = 1 + (rand() % (GRID_WIDTH - 2)); // bottom gate
    gridMap->s = gates[3];
    gates[4] = 1 + (rand() % (GRID_HEIGHT - 2));
    gates[5] = GRID_WIDTH - 1; // right gate
    gridMap->e = gates[4];
    gates[6] = 0;
    gates[7] = 1 + (rand() % (GRID_WIDTH - 2)); // top gate
    gridMap->n = gates[7];
    for (int i = 0; i < 8; i += 2)
    {
        gridMap->map[gates[i]][gates[i + 1]] = pathTile.ascii;
    }
}

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[38;5;137m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define BRN "\e[38;5;94m"
#define LBL "\e[38;5;33m"

void print_grid(terrain_map_t *gridMap)
{
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            char *color = "";
            switch (gridMap->map[i][j])
            {
            case '%':
                color = MAG;
                break;
            case '.':
                color = YEL;
                break;
            case '~':
                color = BLU;
                break;
            case '^':
                color = GRN;
                break;
            case ':':
                color = GRN;
                break;
            case '#':
                color = BRN;
                break;
            case 'C':
                color = RED;
                break;
            case 'M':
                color = LBL;
                break;
            default:
                // unreachable
                fprintf(stderr, "unhandled color case\n");
                break;
            }
            printf("%s%c", color, gridMap->map[i][j]);
        }
        putchar('\n');
    }
}

void gen_voronoi_seeds(Seed storedSeeds[SEED_NUM])
{
    for (int i = 0; i < SEED_NUM; i++)
    {
        storedSeeds[i].x = 1 + (rand() % (GRID_HEIGHT - 1));
        storedSeeds[i].y = 1 + (rand() % (GRID_WIDTH - 1));
        storedSeeds[i].tile = tiles[i % TileCount]->ascii;
    }
    // TODO: above make seed.tile not loop over tiles and have probabilities?
}

static inline double distance(int x, int y, int x2, int y2)
{
    return sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
}

void voronoi_world_gen(Seed storedSeeds[SEED_NUM], terrain_map_t *gridMap)
{
    // start at one because of border
    for (int x = 1; x < GRID_HEIGHT - 1; x++)
    {
        for (int y = 1; y < GRID_WIDTH - 1; y++)
        {
            int closestSeed = 0;
            double closestDist = distance(x, y, storedSeeds[0].x, storedSeeds[0].y);
            for (int i = 1; i < SEED_NUM; i++)
            {
                double currDist = distance(x, y, storedSeeds[i].x, storedSeeds[i].y);
                if (currDist < closestDist)
                {
                    closestSeed = i;
                    closestDist = currDist;
                }
            }
            // TODO: this is not desired way to do this but it works
            // prolly write a switch statement instead
            if (storedSeeds[closestSeed].tile == treeTile.ascii)
            {
                int treeChance = rand() % 100;
                if (treeChance < 25)
                {
                    gridMap->map[x][y] = treeTile.ascii;
                }
                else
                {
                    gridMap->map[x][y] = clearingTile.ascii;
                }
            }
            else
            {
                gridMap->map[x][y] = storedSeeds[closestSeed].tile;
            }
        }
    }
}

void generate_path(terrain_map_t *m, int leftX, int leftY, int botX, int botY, int rightX, int rightY, int topX, int topY)
{

    
    int direction;

    // pick one of the 3 "runs" for placing a pokemart
    int PCplacement = 1 + rand() % 3;
    int PCx = 0, PCy = 0;
    int Mplacement = 1 + rand() % 3;
    int Mx = 0, My = 0;
    // place pokemon center on left run to bottom gate
    if (PCplacement == 1) {
        if(leftX -1 != 0){
        PCx = leftX - 1; // TODO: fixme
        } else if (leftX +1 != 21){
            PCx = leftX +1;
        }
        PCy = leftY + (rand() % (botY - leftY - 1));
       // printf("placement%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }
   
    
    // left gate: go right until bottom gate
    while (leftY < botY)
    {
        m->map[leftX][leftY] = pathTile.ascii;
        leftY++;
    }

    // place pokemon center on virtical run to right gate
    if (PCplacement == 2) {
        if((leftX-rightX) == 0) {
            PCx = leftX;
        }
        else if( (leftX-rightX) < 0 ) {
            PCx = leftX + (rand() % (rightX-leftX));
        } 
        else {
            PCx = rightX + (rand() % (leftX-rightX));
        }
        PCy = leftY - 1;
      //  printf("placement:%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }
   

    direction = leftX > rightX ? -1 : 1;
    // left gate: go in virtical direction of right gate
    while (leftX != rightX)
    {
        m->map[leftX][leftY] = pathTile.ascii;
        leftX += direction;
    }
    
    // place pokemon center on last run to right gate
    if(PCplacement == 3) {
        PCx = leftX - 1; // TODO: fixme
        PCy = leftY + (rand() % (rightY - leftY + 1));
       // printf("placement:%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }

    // left gate: go rest of way in horizontal to right gate
    while (leftY != GRID_WIDTH)
    {
        m->map[leftX][leftY] = pathTile.ascii;
        leftY++;
    }

    // pokemart first "run" bottom up to right gate height
    if(Mplacement == 1){
        // TODO: delete these dogshit if else if
        if(botY -1 != 0){
            My = botY - 1; // TODO: fixme
        } else if (botY +1 != GRID_WIDTH){
            My = botY +1;
        }
        Mx = leftX + (rand() % (botX - leftX + 1));
       // printf("placement%d, Mx:%d, My:%d\n", Mplacement, Mx, My); 
    }

    // bottom gate: go up until right gate (leftX has been changed)
    while (botX > leftX)
    {
        m->map[botX][botY] = pathTile.ascii;
        botX--;
    }

    // place mart on horizontal "run" of bot->top path
    if(Mplacement == 2) {
        if((botY - topY) == 0) {
            My = botY;
        }
        else if((botY-topY) < 0 ) {
            My = botY + (rand() % (topY-botY));
        } 
        else {
            My = topY + (rand() % (botY-topY));
        }
        Mx = botX + 1;
       // printf("placement:%d, Mx:%d, My:%d\n", Mplacement, Mx, My); 
    }

    // bottom gate: go in horizontal direction of top gate
    direction = botY > topY ? -1 : 1;
    while (botY != topY)
    {
        m->map[botX][botY] = pathTile.ascii;
        botY += direction;
    }
    Mplacement = 3;
    // place pokemon center on last run to top gate
    if(Mplacement == 3) {
        My = botY + 1; // TODO: fixme
        Mx = 1 + (rand() % (botX - topX + 1));
     //   printf("placement:%d, Mx:%d, My:%d\n", Mplacement, Mx, My); 
    }


    // bottom gate: go rest of the way up to top gate
    while (botX != 0)
    {
        m->map[botX][botY] = pathTile.ascii;
        botX--;
    }

    m->map[PCx][PCy] = pokemonCenter.ascii;
    m->map[Mx][My] = pokeMart.ascii;
}


void display_map(terrain_map_t *map)
{
 
        Seed seeds[SEED_NUM];
        init_world(map);
        gen_voronoi_seeds(seeds);
        voronoi_world_gen(seeds, map);
        generate_path(map, gates[0], gates[1], gates[2], gates[3], gates[4], gates[5], gates[6], gates[7]);
        print_grid(map);
}

void new_map(terrain_map_t*map)
{
    Seed seeds[SEED_NUM];
    init_world(map);
    gen_voronoi_seeds(seeds);
    voronoi_world_gen(seeds, map);
    generate_path(map, gates[0], gates[1], gates[2], gates[3], gates[4], gates[5], gates[6], gates[7]);

}


int main(void)
{
    terrain_map_t *worldMap[WORLD_HEIGHT][WORLD_WIDTH];
    int currentX = 200; int currentY = 200;
    srand(time(NULL));
    //Seed seeds[SEED_NUM];
    terrain_map_t d;

    for(int i =0; i < WORLD_HEIGHT; i++){
        for(int j =0; j < WORLD_WIDTH; ++j){
            worldMap[i][j] = (terrain_map_t*)malloc(sizeof(terrain_map_t));
        }
    }

    new_map(&d);


    display_map(worldMap[currentX][currentY]);

   // printf("prior to the center call");
   // place_center(&d);
    //print_grid(&d);


    // for(int i =0; i < WORLD_HEIGHT; ++i){
    //     for(int j = 0; i < WORLD_WIDTH; ++j){
    //         free(sizeof(worldMap[i][j]));
    //     }
    // }
    
    return 0;
}
