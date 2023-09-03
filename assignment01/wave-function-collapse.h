#ifndef _WAVE_FUNC_COLLAPSE
#define _WAVE_FUNC_COLLAPSE

#include <tiles.h>


// for further implimentation refer to 
// https://github.com/mxgmn/WaveFunctionCollapse

/*
 * Initalize grid so each tile has the same entropy 
 */
void init_grid();

/*
 * Select the tile with the lowest entropy and collapse its state
 */
void collapse();

/*
 * Propigate the grid reducing the entroy of tiles after collapse
 */
int propigate();



#endif /* _WAVE_FUNC_COLLAPSE */
