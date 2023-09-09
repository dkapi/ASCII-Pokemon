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
            // do something similar to printing the colors
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

    int randB = rand() % 3 + 1;
    int randCM = rand() % 2 + 1;

    int direction;

    // left gate: go right until bottom gate
    while (leftY < botY)
    {
        gridMatrix[leftX][leftY] = pathTile.ascii;
        leftY++;
    }

    // spawning a pokemon center
    if (randB == 1)
    {
        int randDist = rand() % botY + 1;
        if (leftX - 1 != 0 && randCM == 1)
        {
            gridMatrix[leftX - 1][randDist] = pokemonCenter.ascii;
        }
        else
        {
            if (leftX + 1 != 21 && randCM == 1)
            {
                gridMatrix[leftX + 1][randDist] = pokemonCenter.ascii;
            }
        }
    }
    else
    {
        if (randB == 2)
        {
            int randDist = rand() % leftY + 1;
            if (leftY + 1 != 0 && randCM == 1)
            {
                gridMatrix[randDist][leftY] = pokemonCenter.ascii;
            }
            else
            {
                if (leftY - 1 != 79 && randCM == 1)
                {
                    gridMatrix[randDist][leftY] = pokemonCenter.ascii;
                }
            }
        }

        else
        {
            if (randB == 3)
            {
                int randDist = rand() % leftX + 1;
                if (leftY + 1 > leftX && randCM == 1)
                {
                    gridMatrix[randDist][leftX + 1] = pokemonCenter.ascii;
                }
                else
                {
                    if (leftY - 1 != 79 && randCM == 1)
                    {
                        gridMatrix[randDist][leftX - 1] = pokemonCenter.ascii;
                    }
                }
            }
        }
    }

    // left gate: go in virtical direction of right gate
    direction = leftX > rightX ? -1 : 1;
    while (leftX != rightX)
    {
        gridMatrix[leftX][leftY] = pathTile.ascii;
        leftX += direction;
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
