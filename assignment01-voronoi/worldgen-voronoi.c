#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "tiles.h"
#include <math.h>


#define GRID_HEIGHT 24-3
#define GRID_WIDTH 80
#define SEED_NUM 20

static char gridMatrix[GRID_HEIGHT][GRID_WIDTH];
static int leftX, leftY, topX, topY, botX, botY, rightX, rightY;

typedef struct Seed {
    unsigned int x;
    unsigned int y;
    char tile;
} Seed;


void init_world() 
{   
    for(int i=0; i < GRID_HEIGHT; i++) {
       	gridMatrix[i][0] = mountainTile.ascii;
	    gridMatrix[i][GRID_WIDTH-1] =  mountainTile.ascii;
    }
    // TODO maybe better way to impliment this but oh well/ or move to its own func
    topX = 1 + (rand() % (GRID_HEIGHT-1));
    topY = 0;
    gridMatrix[topX][topY] = path.ascii;
    botX = 1 + (rand() % (GRID_HEIGHT-1));
    botY = GRID_WIDTH - 1;
    gridMatrix[botX][botY] = path.ascii;
    
    for(int j=0; j < GRID_WIDTH; j++) {
       gridMatrix[0][j] = mountainTile.ascii;
       gridMatrix[GRID_HEIGHT-1][j] =  mountainTile.ascii;
    }

    leftX = 0;
    leftY = 1 +(rand() % (GRID_WIDTH -1));
    gridMatrix[leftX][leftY] = path.ascii;
    rightX = GRID_HEIGHT -1;
    rightY = 1 + (rand() % (GRID_WIDTH -1));
    gridMatrix[rightX][rightY] = path.ascii;


   for(int i=1; i < GRID_HEIGHT - 1; i++) {
        for(int j=1; j < GRID_WIDTH - 1; j++) {
	    gridMatrix[i][j] =  clearing.ascii;
        }
   }
}

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define BRN "\e[38;5;166m"

void print_grid()
{
    for(int i=0; i < GRID_HEIGHT; i++) {
        for(int j=0; j < GRID_WIDTH; j++) {
            char* color = "";
            switch (gridMatrix[i][j]) {
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
    for(int i=0; i < SEED_NUM; i++){
        storedSeeds[i].x = 1 + (rand() % (GRID_HEIGHT-1));
        storedSeeds[i].y = 1 + (rand() % (GRID_WIDTH-1));
        storedSeeds[i].tile = tiles[i % TileCount]->ascii;
    }
    // TODO: above make seed.tile not loop over tiles and have probabilities?
}

static inline double distance(int x, int y, int x2, int y2)
{
    return sqrt( (x-x2) * (x-x2) + (y-y2) * (y-y2));
}

void voronoi_world_gen(char gridMatrix[GRID_HEIGHT][GRID_WIDTH], Seed storedSeeds[SEED_NUM])
{   
    // start at one because of border
    for (int x =1; x < GRID_HEIGHT - 1 ; x++){
        for ( int y =1; y < GRID_WIDTH - 1; y++){
            int closestSeed = 0;
            double closestDist = distance(x , y, storedSeeds[0].x, storedSeeds[0].y);
            for(int i=1; i < SEED_NUM; i++){
                double currDist = distance(x , y, storedSeeds[i].x, storedSeeds[i].y);
                if(currDist < closestDist){
                    closestSeed = i;
                    closestDist = currDist;
                }
            }
            // TODO: this is not desired way to do this but it works
            // do something similar to printing the colors
            if(storedSeeds[closestSeed].tile == '^') {
                int treeChance = rand() % 100;
                if (treeChance < 25) {
                    gridMatrix[x][y] = treeTile.ascii;
                }
                // else if(25 < treeChance && treeChance < 50) {
                //     gridMatrix[x][y] = tallGrass.ascii;
                // }
                else {
                    gridMatrix[x][y] = clearing.ascii;
                }
            }
            else {
                gridMatrix[x][y] = storedSeeds[closestSeed].tile;
            }
        }
     }
}

void generate_path(char grid[GRID_HEIGHT][GRID_WIDTH], int leftGateX, int leftGateY, int rightGateX, int rightGateY,
                    int topGateX, int topGateY, int botGateX, int botGateY) 
{   // x is y, y should be x
    int x = leftGateX;
    int y = leftGateY;
    int j = botGateX;
    int k = botGateY;

    while (y <= topGateY) {
        grid[x][y] = path.ascii;
        y++;
    } y--;

    if( x <= rightGateX){
        while (x <=rightGateX){
            grid[x][y] = path.ascii;
            x++;
        } x--;

    } else if (x >= rightGateX){
        while (x >= rightGateX)
        {
            grid[x][y] = path.ascii;
            x--;
        } x++;
    }

    while (y <= rightGateY){
        grid[x][y] = path.ascii;
        y++;
    }

    // down here is bot to top path looping
    while(j>= rightGateX){
        grid[j][k] = path.ascii;
        j--;
    } j++;

    if( k <= topGateY){
        while(k <= topGateY){
            grid[j][k] = path.ascii;
            k++;
        } k--;
    } else if (k >= topGateY){
        while (k >= topGateY)
        {
            grid[j][k] = path.ascii;
            k--;
        }k++;
    }
    while(j >= topGateX){
        grid[j][k] = path.ascii;
        j--;
    }
}


int main(void)
{
    srand(time(NULL));    
    Seed seeds[SEED_NUM];

    gen_voronoi_seeds(seeds);
    init_world();
    voronoi_world_gen(gridMatrix, seeds);

    // okay so my grid is like inversed so top is left, bot is right...etc :)
    generate_path(gridMatrix, topX, topY, botX, botY, leftX, leftY, rightX, rightY);
    print_grid();
    
    return 0;
}
