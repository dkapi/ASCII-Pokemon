#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include "worldgen.h"
#include "voronoi.h"

void print_map(terrain_map_t *map) // TODO: take grid not map
{
    for(int i = 0; i < GRID_HEIGHT; i++) {
        for(int j = 0; j < GRID_WIDTH; j++) {
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

static inline void get_input(int n, char* buf)
{
    fgets(buf, n, stdin);
}

Location_t handle_input(int n, char* buf, Location_t loc, Gates_t* gates, bool* quit)
{
    switch((int)buf[0]) {
        case 'n':
            loc.y += loc.y > 0 ? 1 : 0;
            gates->bottom = (Location_t) { .x = gates->bottom.x, .y = gates->top.y};
            gates->top = (Location_t){ .x = 0, .y = 0 };
            gates->left = (Location_t){ .x = 0, .y = 0 };
            gates->right = (Location_t){ .x = 0, .y = 0 };
            break;
        case 's':
            loc.y += loc.y < 401 ? -1 : 0;
            gates->top = (Location_t){ .x = gates->top.x, .y = gates->bottom.y };
            gates->bottom = (Location_t){ .x = 0, .y = 0 };
            gates->left = (Location_t){ .x = 0, .y = 0 };
            gates->right = (Location_t){ .x = 0, .y = 0 };
            break;
        case 'e':
            loc.x += loc.x > 0 ? 1 : 0;
            gates->left = (Location_t){ .x = gates->right.x, .y = gates->left.y };
            gates->bottom = (Location_t){ .x = 0, .y = 0 };
            gates->top = (Location_t){ .x = 0, .y = 0 };
            gates->right = (Location_t){ .x = 0, .y = 0 };
            break;
        case 'w':
            loc.x += loc.x < 401 ? -1 : 0;
            gates->right = (Location_t){ .x = gates->left.x, .y = gates->right.y };
            gates->bottom = (Location_t){ .x = 0, .y = 0 };
            gates->top = (Location_t){ .x = 0, .y = 0 };
            gates->left = (Location_t){ .x = 0, .y = 0 };
            break;
        case 'f':
            int x,y;
            sscanf(buf, "f %d %d", &x, &y);
            if(x < 401 && y < 401) {
                loc.x = x;
                loc.y = y;
                *gates = (Gates_t){0};
            }
            break;
        case 'q':
            *quit = true;
            break;
    }
    return loc;
}


int main(void)
{
    srand(time(NULL));
    terrain_map_t* worldMap[WORLD_HEIGHT][WORLD_WIDTH];

    for(int i = 0; i < 401; ++i) {
        for(int j = 0; j < 401; ++j) {
            worldMap[i][j] = NULL;
        }
    }

    terrain_map_t* currGrid = (terrain_map_t*)malloc(sizeof(terrain_map_t));
    Location_t currLoc = { .x = 200, .y = 200 };
    currGrid->location = currLoc;
    Gates_t gates = {0};
    generate_voronoi_map(currGrid, gates);
    worldMap[currGrid->location.x][currGrid->location.y] = currGrid;
    gates = currGrid->gates;
    print_map(worldMap[currLoc.x][currLoc.y]);
        printf("%scurrent location: (%d,%d) movement input:",WHT,currLoc.x-200,currLoc.y-200 );


    char userInput[10];
    bool shouldQuit = false;
    while(!shouldQuit) {
        size_t input_n = sizeof(userInput)/sizeof(userInput[0]);
        get_input(input_n, &userInput[0]);
        Location_t newLoc = handle_input(input_n, &userInput[0], currLoc, &gates, &shouldQuit);
        if((currLoc.x != newLoc.x || currLoc.y != newLoc.y) && worldMap[newLoc.x][newLoc.y] == NULL) {
            terrain_map_t* newGrid = (terrain_map_t*)malloc(sizeof(terrain_map_t));
            newGrid->location = newLoc;
            generate_voronoi_map(newGrid, gates);
            worldMap[newGrid->location.x][newGrid->location.y] = newGrid;
            currGrid = newGrid;
            
            // printf("%s curr location: (%d, %d)\n",WHT, currLoc.x-200, currLoc.y-200);
        }
        print_map(worldMap[newLoc.x][newLoc.y]);
        printf("%scurrent location: (%d,%d) movment input:",WHT,newLoc.x-200,newLoc.y-200 );
        printf(WHT);
        currLoc = newLoc;
        gates = currGrid->gates;

    }

    for(int i =0; i < WORLD_HEIGHT; i++) {
        for(int j =0; j < WORLD_WIDTH; j++) {
            if(worldMap[i][j] != NULL){
            free(worldMap[i][j]);
            }
        }
    }

    return 0;
}
