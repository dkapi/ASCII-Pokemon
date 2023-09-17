#include <stdio.h>
#include <time.h>
#include "worldgen.h"
#include "vorinoi.h"

void print_map(terrain_map_t *map)
{
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            char *color = "";
            switch (map->grid[i][j]) {
                case '%':
                    color = MAG;
                    break;
                case '.':
                    color = YEL;
                    break;
                case '~':
                    color = BLU;
                    break;
                case '^':
                    color = GRN;
                    break;
                case ':':
                    color = GRN;
                    break;
                case '#':
                    color = BRN;
                    break;
                case 'C':
                    color = RED;
                    break;
                case 'M':
                    color = LBL;
                    break;
                default:
                    // unreachable
                    fprintf(stderr, "unhandled color case\n");
                    break;
            }
            printf("%s%c", color, map->grid[i][j]);
        }
        putchar('\n');
    }
}


int main(void)
{
    srand(time(NULL));
    terrain_map_t* worldMap[WORLD_HEIGHT][WORLD_WIDTH];
    terrain_map_t* firstGrid = (terrain_map_t*)malloc(sizeof(terrain_map_t));
    Location_t currLoc = { .x = 200, .y = 200 };
    firstGrid->location = currLoc;
    generate_voronoi_map(firstGrid);
    worldMap[firstGrid->location.x][firstGrid->location.y] = firstGrid;
    print_map(worldMap[currLoc.x][currLoc.y]);

    return 0;
}
