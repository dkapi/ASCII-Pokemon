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

typedef struct Seed{
    int x;
    int y;
    char tile;
} Seed;


void init_world() 
{
   for(int i=0; i < GRID_HEIGHT; i++) {
       	gridMatrix[i][0] = mountainTile.ascii;
	gridMatrix[i][GRID_WIDTH-1] =  mountainTile.ascii;
   }

   for(int j=0; j < GRID_WIDTH; j++) {
       gridMatrix[0][j] = mountainTile.ascii;
       gridMatrix[GRID_HEIGHT-1][j] =  mountainTile.ascii;
   }

   for(int i=1; i < GRID_HEIGHT - 1; i++) {
       for(int j=1; j < GRID_WIDTH - 1; j++) {
	   gridMatrix[i][j] =  clearing.ascii;
       }
   }
}

void printGrid()
{
    //	printf("\x1b[H");
    for(int i=0; i < GRID_HEIGHT; i++) {
	for(int j=0; j < GRID_WIDTH; j++) {
	  // 	putchar(gridMatrix[i][j]);
	    if(gridMatrix[i][j] == '%'){
		printf(MAG "%c", gridMatrix[i][j]);
	    }
	    if(gridMatrix[i][j] == '.'){
		printf(YEL "%c", gridMatrix[i][j]);
	    }
        if(gridMatrix[i][j] == '~'){
            printf(BLU "%c", gridMatrix[i][j]);
        }
        if(gridMatrix[i][j] == '^'){
            printf(GRN "%c", gridMatrix[i][j]);
        }
        if(gridMatrix[i][j] == ':'){
            printf(GRN "%c", gridMatrix[i][j]);
        }
	}
	putchar('\n');
    }	       
}


void gen_seeds(Seed storedSeeds[SEED_NUM])
{

    for(int i =0; i < SEED_NUM; i++){
	storedSeeds[i].x = (rand() % GRID_WIDTH-1)+1;
	storedSeeds[i].y = (rand() % GRID_HEIGHT-1)+1;
    }
    storedSeeds[0].tile = mountainTile.ascii;
    storedSeeds[1].tile = treeTile.ascii;
    storedSeeds[2].tile = clearing.ascii;
    storedSeeds[3].tile = tallGrass.ascii;
    storedSeeds[4].tile = water.ascii;
    storedSeeds[5].tile = tallGrass.ascii;
    storedSeeds[6].tile = water.ascii;
    storedSeeds[7].tile = treeTile.ascii;
    storedSeeds[8].tile = mountainTile.ascii;
    storedSeeds[9].tile = clearing.ascii;
    storedSeeds[10].tile = mountainTile.ascii;
    storedSeeds[11].tile = treeTile.ascii;
    storedSeeds[12].tile = clearing.ascii;
    storedSeeds[13].tile = tallGrass.ascii;
    storedSeeds[14].tile = water.ascii;
    storedSeeds[15].tile = tallGrass.ascii;
    storedSeeds[16].tile = water.ascii;
    storedSeeds[17].tile = treeTile.ascii;
    storedSeeds[18].tile = mountainTile.ascii;
    storedSeeds[19].tile = clearing.ascii;
}

double distance(int x, int y, int x2, int y2)
{
    return sqrt(pow(x - x2, 2) + pow(y -y2, 2));
}

void voronoi_world_gen(char gridMatrix[GRID_HEIGHT][GRID_WIDTH], Seed storedSeeds[SEED_NUM])
{
    for (int x =1; x < GRID_HEIGHT -1; x++){
        for ( int y =1; y < GRID_WIDTH -1; y++){
            int closestSeed = 0;
            double closestDistance = distance(x , y, storedSeeds[0].x,
					      storedSeeds[0].y);

            for(int i =1; i < SEED_NUM; i++){
                double d = distance(x , y, storedSeeds[i].x,
				    storedSeeds[i].y);
                if(d < closestDistance){
                    closestSeed = i;
                    closestDistance = d;
                }
            }
            gridMatrix[x][y] = storedSeeds[closestSeed].tile;
        }
     }
}

int main(void)
{
    srand(time(NULL));    
    Seed seeds[SEED_NUM];

    gen_seeds(seeds);
    init_world();
    voronoi_world_gen(gridMatrix, seeds);
    printGrid();
    
//    printf("\x1b[2J"); 
    // for(;;) {
    // 	// printf("\033[?25l");  // hide cursor
    // 	printGrid();
    // 	// printf("\033[?25h"); // show cursor
    // 	usleep(100000);  // 100ms
    // }
    return 0;
}
