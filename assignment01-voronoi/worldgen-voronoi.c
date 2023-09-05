#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "tiles.h"


#define GRID_HEIGHT 24-3
#define GRID_WIDTH 80

static char gridMatrix[GRID_HEIGHT][GRID_WIDTH];
int seed_points[8][2];

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
		    //	putchar(gridMatrix[i][j]);
		    if(gridMatrix[i][j] == '%'){
			printf(MAG "%c", gridMatrix[i][j]);
		    }
		    if(gridMatrix[i][j] == '.'){
			printf(YEL "%c", gridMatrix[i][j]);
		    }
		}
		putchar('\n');
	}	       
}


void gen_seeds()
{
    int minX = 1; int maxX = 79; int minY = 1; int maxY = 20;
    
    for(int i =0; i < 4; i++){
	seed_points[i][0] = rand() % (maxX - minX + 1) + minX;
	seed_points[i][1] = rand() % (maxY - minY + 1) + minY;
    }
}

int main(int argc, char* argv[])
{
    srand(time(NULL));

    gen_seeds();
    init_world();
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
