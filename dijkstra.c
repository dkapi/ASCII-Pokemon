#include "heap.h"
#include "dijkstra.h"
#include "characters.h"
#include <stdio.h>


//compares cost as apposed to sheaffers height map compare
static int32_t character_cost_cmp(const void *key, const void *with) {
    struct character_s * k = (struct character_s*)key;
    struct character_s * w = (struct character_s*)with;
    return k->cost(*k->tile) - w->cost(*w->tile);
}

struct tile_s* char_to_tile_s(char terrain){

    switch(terrain){
        case ':':
            return &tallGrassTile;
        case '^': 
            return &treeTile;
        case '~':
            return &waterTile;
        case '%':
            return &mountainTile;
        case '.':
            return &clearingTile;
        case 'C':
            return &pokemonCenter;
        case 'M':
            return &pokeMart;
        case '#':
            return &pathTile;
        default:
        // unreachable
        fprintf(stderr, "something went wrong in char_to_tile_s func");
        return &pathTile;
    } 
}

void dijkstra_map(terrain_map_t *map, Location_t *start, dijk_map_t *dNode[GRID_HEIGHT][GRID_WIDTH], struct character_s *npc)
{   
    int i, j;
    static uint32_t initialized = 0;
    dijk_map_t *dn = (dijk_map_t *)malloc(sizeof(dijk_map_t));
    heap_t h;

    //allocate mem for Dnode
    for (i = 0; i < GRID_HEIGHT; i++) {
        for (j = 0; j < GRID_WIDTH; j++) {
            dNode[i][j] = (dijk_map_t *)malloc(sizeof(dijk_map_t));
        }
    }

    //initialize the dNode to simulate given grid size, and set stuff
    //crashing here
    if (!initialized) {
        for (i = 0; i < GRID_HEIGHT; i++) {
            for (j = 0; j < GRID_WIDTH; j++) {
                // still failing (seg fault line 32)
                dNode[i][j]->pos.x = i;
                dNode[i][j]->pos.y = j;
                dNode[i][j]->tile =0;
                dNode[i][j]->tile = char_to_tile_s(map->grid[i][j]);
            }
        }
        initialized =1;
    }
    // initialize all costs to INF
    for (i =0; i < GRID_HEIGHT; i++) {
        for (j =0; j < GRID_WIDTH; j++) {
            dNode[i][j]->cost = INT32_MAX;
        }
    }
    // setting cost of player loco to zero
    dNode[start->x][start->y]->cost = 0;

    heap_init(&h, character_cost_cmp, NULL);

    for (i =1; i < GRID_HEIGHT; i++) {
        for( j =1; j < GRID_WIDTH; j++) {
            dNode[i][j]->hn = heap_insert(&h, &dNode[i][j]);
        }
    }
    // okay issue i think is that i dont need dn? and i dont even know wtf im doing with it because its not initialized in main LOL
    while (h.size > 0) {
        dn = (dijk_map_t *)heap_remove_min(&h);
        if (dn->cost == INT32_MAX) {
            break;
        }
    }
    //mark currnode as visited
    dn->visited = 1;
    int dx, dy, neighborX, neighborY;

    for(dx = -1; dx <= 1; dx++) {
        for(dy = -1; dy <= 1; dy++){
            // these values are somehow uninitialized in the if jumps below?
            neighborX = dn->pos.x + dx;
            neighborY = dn->pos.y + dy;

            if(neighborX >= 0 && neighborY <GRID_HEIGHT &&
               neighborY >= 0 && neighborY <GRID_WIDTH){
                dijk_map_t **neighbor = &dNode[neighborX][neighborY];
                // ik this shit wrong, but need to fix other stuff before code compiles to this line
                // this npc->cost is why currently cost map is filled with 47
                uint32_t newCost = dn->cost + npc->cost;

                if(newCost < (*neighbor)->cost) {
                    (*neighbor)->cost = newCost;
                    heap_decrease_key(&h, (*neighbor)->hn, NULL);
                }
            }
        }
    }
    free(dn);
}

void print_dijkstra_map(dijk_map_t *dNode[GRID_HEIGHT][GRID_WIDTH]) {
    int i, j;
    for (i = 0; i < GRID_HEIGHT; i++) {
        for (j = 0; j < GRID_WIDTH; j++) {
            if (dNode[i][j]->cost < 10) {
                printf(" %2d ", dNode[i][j]->cost % 100);
            } else {
                printf("%2d ", dNode[i][j]->cost % 100);
            }
        }
        printf("\n");
    }
}

void dijkstra_free(dijk_map_t *dNode[GRID_HEIGHT][GRID_WIDTH]) {
    (void) pathTile;
    (void) tiles[TileCount];
    
    for (int i = 0; i< GRID_HEIGHT; i++) {
        for (int j =0; j < GRID_WIDTH; j++) {
            free(dNode[i][j]);
        }
    }
}


