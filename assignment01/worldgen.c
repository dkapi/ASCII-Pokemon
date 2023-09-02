#include <stdio.h>

#define GRID_HEIGHT 80
#define GRID_WIDTH 24


int main(int argc, char* argv[])
{
    int gridMatrix[GRID_HEIGHT][GRID_WIDTH];
    
    for(int i =0; i < GRID_HEIGHT; i++){
	gridMatrix[0][i] = 37;
	for(int j =0; j < GRID_WIDTH -3; j++){
	    if(gridMatrix[i][j] != 64) continue;
	    gridMatrix[i][j] = 64;
	    printf("%c", gridMatrix[i][j]);
	}
    }
}
