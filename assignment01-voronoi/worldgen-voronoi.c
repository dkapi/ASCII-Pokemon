#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "tiles.h"
#include <math.h>

#define GRID_HEIGHT 24 - 3
#define GRID_WIDTH 80
#define SEED_NUM 20

static char gridMatrix[GRID_HEIGHT][GRID_WIDTH];
static int gates[4 * 2]; // (4 x,y pairs for gates)

typedef struct Seed
{
    unsigned int x;
    unsigned int y;
    char tile;
} Seed;

void init_world()
{
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        gridMatrix[i][0] = mountainTile.ascii;
        gridMatrix[i][GRID_WIDTH - 1] = mountainTile.ascii;
    }

    for (int j = 0; j < GRID_WIDTH; j++)
    {
        gridMatrix[0][j] = mountainTile.ascii;
        gridMatrix[GRID_HEIGHT - 1][j] = mountainTile.ascii;
    }

    for (int i = 1; i < GRID_HEIGHT - 1; i++)
    {
        for (int j = 1; j < GRID_WIDTH - 1; j++)
        {
            gridMatrix[i][j] = clearingTile.ascii;
        }
    }

    // these may seem "backwards" but x gives us row and y gives entry in that row
    gates[0] = 1 + (rand() % (GRID_HEIGHT - 2));
    gates[1] = 0; // left gate
    gates[2] = GRID_HEIGHT - 1;
    gates[3] = 1 + (rand() % (GRID_WIDTH - 2)); // bottom gate
    gates[4] = 1 + (rand() % (GRID_HEIGHT - 2));
    gates[5] = GRID_WIDTH - 1; // right gate
    gates[6] = 0;
    gates[7] = 1 + (rand() % (GRID_WIDTH - 2)); // top gate
    for (int i = 0; i < 8; i += 2)
    {
        gridMatrix[gates[i]][gates[i + 1]] = pathTile.ascii;
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

void print_grid()
{
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            char *color = "";
            switch (gridMatrix[i][j])
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
                color = BLU;
                break;
            default:
                // unreachable
                fprintf(stderr, "unhandled color case\n");
                break;
            }
            printf("%s%c", color, gridMatrix[i][j]);
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

void voronoi_world_gen(Seed storedSeeds[SEED_NUM])
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
                    gridMatrix[x][y] = treeTile.ascii;
                }
                else
                {
                    gridMatrix[x][y] = clearingTile.ascii;
                }
            }
            else
            {
                gridMatrix[x][y] = storedSeeds[closestSeed].tile;
            }
        }
    }
}

void generate_path(int leftX, int leftY, int botX, int botY, int rightX, int rightY, int topX, int topY)
{

    
    int direction;

    // pick one of the 3 "runs" for placing a pokemart
    int PCplacement = 1 + rand() % 3;
    int PCx = 0, PCy = 0;
    int Mplacement = 1 + rand() %3;
    int Mx = 0, My = 0;
    // place pokemon center on left run to bottom gate
    if (PCplacement == 1) {
        if(leftX -1 != 0){
        PCx = leftX - 1; // TODO: fixme
        } else if (leftX +1 != 21){
            PCx = leftX +1;
        }
        PCy = leftY + (rand() % (botY - leftY - 1));
        printf("placement%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }
    //pokemart placement if 1
    if(Mplacement == 1){
        if( leftX - 1 != 0){
            Mx = leftX -1;
        } else if (leftX+1 != 21){
            Mx = leftX +1;
        }
        Mx = leftY + (rand() % (botY - leftY -1));
    }
    
    // left gate: go right until bottom gate
    while (leftY < botY)
    {
        gridMatrix[leftX][leftY] = pathTile.ascii;
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
        printf("placement:%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }
    // place mart if 2
    if(Mplacement == 2) {
        if((leftX - rightX) == 0){
            Mx = leftX;
        }
        else if ((leftX-rightX) < 0 ) {
            Mx = leftX + (rand() % (rightX - leftX));
        }
        else {
            Mx = rightX + (rand() % (leftX-rightX));
        }
        My = leftY -1;
    }

    direction = leftX > rightX ? -1 : 1;
    // left gate: go in virtical direction of right gate
    while (leftX != rightX)
    {
        gridMatrix[leftX][leftY] = pathTile.ascii;
        leftX += direction;
    }
    
    // place pokemon center on last run to right gate
    if(PCplacement == 3) {
        PCx = leftX - 1; // TODO: fixme
        PCy = leftY + (rand() % (rightY - leftY + 1));
        printf("placement:%d, PCx:%d, PCy:%d\n", PCplacement, PCx, PCy); 
    }

    if(Mplacement == 3) {
        Mx = leftX -1;
        My = leftY + (rand() % (rightY - leftY +1));
    }

    // left gate: go rest of way in horizontal to right gate
    while (leftY != GRID_WIDTH)
    {
        gridMatrix[leftX][leftY] = pathTile.ascii;
        leftY++;
    }

    // bottom gate: go up until right gate (leftX has been changed)
    while (botX > leftX)
    {
        gridMatrix[botX][botY] = pathTile.ascii;
        botX--;
    }
    // bottom gate: go in horizontal direction of top gate
    direction = botY > topY ? -1 : 1;
    while (botY != topY)
    {
        gridMatrix[botX][botY] = pathTile.ascii;
        botY += direction;
    }
    // bottom gate: go rest of the way up to top gate
    while (botX != 0)
    {
        gridMatrix[botX][botY] = pathTile.ascii;
        botX--;
    }
    gridMatrix[PCx][PCy] = pokemonCenter.ascii;
    gridMatrix[Mx][My] = pokeMart.ascii;
}

int main(void)
{
    srand(time(NULL));
    Seed seeds[SEED_NUM];

    gen_voronoi_seeds(seeds);
    init_world();
    voronoi_world_gen(seeds);

    generate_path(gates[0], gates[1], gates[2], gates[3], gates[4], gates[5], gates[6], gates[7]);
    print_grid();

    return 0;
}
