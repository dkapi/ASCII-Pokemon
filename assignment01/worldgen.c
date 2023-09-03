#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "wave-function-collapse.h"


static char gridMatrix[GRID_HEIGHT][GRID_WIDTH];


// TODO: remove after wfc?
// or just make wfc take in that all egdes are mountains for now
void init_world() 
{
   for(int i=0; i < GRID_HEIGHT; i++) {
       	gridMatrix[i][0] = mountainTile.ascii;
	gridMatrix[i][GRID_WIDTH-1] = mountainTile.ascii;
   }

   for(int j=0; j < GRID_WIDTH; j++) {
       gridMatrix[0][j] = mountainTile.ascii;
       gridMatrix[GRID_HEIGHT-1][j] = mountainTile.ascii;
   }

   for(int i=1; i < GRID_HEIGHT - 1; i++) {
       for(int j=1; j < GRID_WIDTH - 1; j++) {
	   gridMatrix[i][j] = clearing.ascii;
       }
   }
}

void printGrid()
{
//	printf("\x1b[H");
	for(int i=0; i < GRID_HEIGHT; i++) {
		for(int j=0; j < GRID_WIDTH; j++) {
			putchar(gridMatrix[i][j]);
		}
		putchar('\n');
    }
}

int main(int argc, char* argv[])
{
	init_world();
	printGrid();
//	printf("\x1b[2J"); 
	// for(;;) {
	// 	// printf("\033[?25l");  // hide cursor
	// 	printGrid();
	// 	// printf("\033[?25h"); // show cursor
	// 	usleep(100000);  // 100ms
	// }
    return 0;
}
