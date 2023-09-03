#include "common.h"
#include "wave-function-collapse.h"


// we need a 3rd dimension to represent the superposition of each tile
static int grid[GRID_HEIGHT][GRID_WIDTH][TileCount]; 


void init_grid()
{
    for(int i=0; i < GRID_HEIGHT; i++) {
        for(int j=0; j < GRID_WIDTH; j++) {
            for (int tState=0; tState < TileCount; tState++) {
                // we are setting to tState since our enum starts at 0 and does not have gaps
                grid[i][j][tState] = tState;
            } 
        }
    }
}

// TODO: having probability of what tile collapses is difficult?
void collapse()
{
    // 1. Find lowest entropy (if tied pick at random)

    // 2. Collapse to a tile state

    // 3. Propigate tiles to be updated from collapse

    // 4. Repeat 1-3 until completed grid
}