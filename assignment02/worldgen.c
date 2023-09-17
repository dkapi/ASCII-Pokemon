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

    for(int i = 0; i < 401; ++i){
        for(int j = 0; j < 401; ++j){
            worldMap[i][j] = NULL;
        }
    }

    terrain_map_t* firstGrid = (terrain_map_t*)malloc(sizeof(terrain_map_t));
    Location_t currLoc = { .x = 200, .y = 200 };
    firstGrid->location = currLoc;
    generate_voronoi_map(firstGrid);
    worldMap[firstGrid->location.x][firstGrid->location.y] = firstGrid;
    print_map(worldMap[currLoc.x][currLoc.y]);
    printf(WHT);

    terrain_map_t* nextGrid;
    char userInput[10];
    while(1){

        fgets(userInput,sizeof(userInput), stdin);
        switch(userInput[0]){
            case 'n':
            if(currLoc.y>0){
                currLoc.y++;
                if(worldMap[currLoc.x][currLoc.y] == NULL){
                    nextGrid = 
                    (terrain_map_t*)malloc(sizeof(terrain_map_t));
                    nextGrid->location= currLoc;
                    generate_voronoi_map(nextGrid);
                }
                worldMap[nextGrid->location.x][nextGrid->location.y]
                = nextGrid;
                print_map(worldMap[currLoc.x][currLoc.y]);
                printf("%s curr location: (%d, %d)\n",WHT, currLoc.x-200, currLoc.y-200);
            }
            break;
            case 's':
            if(currLoc.y<401){
                currLoc.y--;
                if(worldMap[currLoc.x][currLoc.y] == NULL){
                    nextGrid = 
                    (terrain_map_t*)malloc(sizeof(terrain_map_t));
                    nextGrid->location= currLoc;
                    generate_voronoi_map(nextGrid);
                }
                worldMap[nextGrid->location.x][nextGrid->location.y]
                = nextGrid;
                print_map(worldMap[currLoc.x][currLoc.y]);
                printf("%s curr location: (%d, %d)\n",WHT, currLoc.x-200, currLoc.y-200);
            }
            break;
            case 'e':
            if(currLoc.x >0){
                currLoc.x--;
                if(worldMap[currLoc.x][currLoc.y] == NULL){
                    nextGrid = 
                    (terrain_map_t*)malloc(sizeof(terrain_map_t));
                    nextGrid->location= currLoc;
                }
                generate_voronoi_map(nextGrid);
                worldMap[nextGrid->location.x][nextGrid->location.y]
                = nextGrid;
                print_map(worldMap[currLoc.x][currLoc.y]);
                printf("%s curr location: (%d, %d)\n",WHT, currLoc.x-200, currLoc.y-200);
            }
            break;
            case 'w':
            if(currLoc.x <401){
                currLoc.x++;
                if(worldMap[currLoc.x][currLoc.y] == NULL){
                    nextGrid = 
                    (terrain_map_t*)malloc(sizeof(terrain_map_t));
                    nextGrid->location= currLoc;
                }
                generate_voronoi_map(nextGrid);
                worldMap[nextGrid->location.x][nextGrid->location.y]
                = nextGrid;
                print_map(worldMap[currLoc.x][currLoc.y]);
                printf("%s curr location: (%d, %d)\n",WHT, currLoc.x-200, currLoc.y-200);
            }
            break;
            case 'f':
            int x,y;
            sscanf(userInput, "f %d %d", &x, &y);
            if(x <401 && y < 401){
                printf("logicX: %d logicY: %d", x, y);
                currLoc.x = x;
                currLoc.y = y;
                printf("curx: %d currY %d", currLoc.x, currLoc.y);

                if(worldMap[currLoc.x][currLoc.y] == NULL){
                    nextGrid = 
                    (terrain_map_t*)malloc(sizeof(terrain_map_t));
                    nextGrid->location= currLoc;
                    generate_voronoi_map(nextGrid);
                
                worldMap[nextGrid->location.x][nextGrid->location.y]
                = nextGrid;
                print_map(worldMap[currLoc.x][currLoc.y]);
                printf("%s curr location: (%d, %d)\n",WHT, currLoc.x-200, currLoc.y-200);
                }
            }
            break;
            case 'q':
            //quit program goto cleanup free mem
            goto cleanup;
        default:
            continue;
        }
    }

    cleanup:
        for(int i =0; i < WORLD_HEIGHT; i++){
            for(int j =0; j < WORLD_WIDTH; j++){
                if(worldMap[i][j] != NULL){
                free(worldMap[i][j]);
               // worldMap[i][j] = NULL;
                }
            }
        }

    return 0;
}
