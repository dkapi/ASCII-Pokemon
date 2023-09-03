#include "common.h"
#include "wave-function-collapse.h"


// we need a 3rd dimension to represent the superposition of each tile
static struct tile_s* grid[GRID_HEIGHT][GRID_WIDTH][TileCount]; 


// INFO: https://github.com/mxgmn/WaveFunctionCollapse
// above is the github for a good example of wfc refer to it for help

// if each edge tile is known, set that here dino
void init_grid()
{
    // set each tile with the values in tiles (in tiles.h)
}


int propigate(int row, int col)
{
   // from here add the 4 neighbors and reduce their state, propigate further if required
   // when lowering the entropy of states set the invalid index to null 
}


// TODO: having probability of what tile collapses is difficult?
void collapse(int* row, int* col, char* ascii)
{
    // 1. Find lowest entropy (if tied pick at random)

    // 2. Collapse to a tile state

    // 3. Propigate tiles to be updated from collapse

    // 4. Repeat 1-3 until completed grid
}