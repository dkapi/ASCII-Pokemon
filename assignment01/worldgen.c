#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "wave-function-collapse.h"


static char gridMatrix[GRID_HEIGHT][GRID_WIDTH];


// TODO: remove after wfc?
// or just make wfc take in that all egdes are mountains for now
void init_world() 
{
	
}

void printGrid()
{
	printf("\x1b[H");
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
	printf("\x1b[2J"); 
	// for(;;) {
	// 	// printf("\033[?25l");  // hide cursor
	// 	printGrid();
	// 	// printf("\033[?25h"); // show cursor
	// 	usleep(100000);  // 100ms
	// }
    return 0;
}
