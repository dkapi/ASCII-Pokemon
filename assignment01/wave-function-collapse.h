#ifndef _WAVE_FUNC_COLLAPSE
#define _WAVE_FUNC_COLLAPSE

/*
 * Initalize grid so each tile has the same entropy 
 */
void init_grid();

/*
 * Propigate the grid reducing the entroy of tiles after collapse
 * @param row, col: position selected to propigate from (ie the collapsed position)
 */
int propigate(int row, int col);

/* Select the tile with the lowest entropy and collapse its state
 * @param row, col: the position for the tile that was collapsed
 * @param ascii: the character that represents the tile that was collapsed
 */
void collapse(int* row, int* col, char* ascii);



#endif /* _WAVE_FUNC_COLLAPSE */
