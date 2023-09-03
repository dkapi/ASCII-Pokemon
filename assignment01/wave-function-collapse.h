#ifndef _WAVE_FUNC_COLLAPSE
#define _WAVE_FUNC_COLLAPSE

/*
 * Initalize grid so each tile has the same entropy 
 */
void init_grid();

/*
 * Propigate the grid reducing the entroy of tiles after collapse
 */
int propigate(int row, int col);

/* Select the tile with the lowest entropy and collapse its state
 * @param row, collumn: the position for the tile that was collapsed
 * @param ascii: the character that represents the tile that was collapsed
 */
void collapse(int* row, int* col, char* ascii);



#endif /* _WAVE_FUNC_COLLAPSE */
