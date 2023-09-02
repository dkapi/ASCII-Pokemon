#include <stdio.h>
#include <stdint.h>

#define GRID_HEIGHT 24
#define GRID_WIDTH 80

static int8_t gridMatrix[GRID_HEIGHT][GRID_WIDTH];

void init_world() 
{
	// TODO: impliment initalization of tiles
}

int main(int argc, char* argv[])
{

	init_world();

    for(int i=0; i < GRID_HEIGHT - 3; i++) {
		for(int j=0; j < GRID_WIDTH; j++) {
			gridMatrix[i][j] = 64;
			printf("%c", gridMatrix[i][j]);
		}
		printf("\n");
    }
}
