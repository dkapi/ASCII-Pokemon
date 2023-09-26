#include "heap.h"
#include "dijkstra.h"
#include "characters.h"
#include <stdio.h>


//compares cost as apposed to sheaffers height map compare
static int32_t cost_cmp(const void *key, const void *with) {
  return ((struct character_s *)key)->cost - ((struct character_s * )with)-> cost;
}

// static int32_t edge_penalty(int8_t x, int8_t y)
// {
//   return (x == 1 || y == 1 || x == GRID_WIDTH - 2 || y == GRID_HEIGHT - 2) ? 2 : 1;
// }

// static int32_t isValid(int8_t x, int8_t y) {
//     return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
// }

void dijkstra_map(terrain_map_t *map, Location_t *start, dijk_map_t *dNode[GRID_WIDTH][GRID_HEIGHT], dijk_map_t *dn, struct character_s *npc)
{   
    int i, j;
    static uint32_t initialized = 0;
    heap_t h;

    //initialize the dNode to simulate given grid size, and set stuff
    //crashing here
    if (!initialized) {
        for (i = 0; i < GRID_WIDTH; i++) {
            for (j = 0; j < GRID_HEIGHT; j++) {
                dNode[i][j]->pos->x = i;
                dNode[i][j]->pos->y = j;
                dNode[i][j]->visited =0;
                dNode[i][j]->hn = NULL;
            }
        }
        initialized =1;
    }
    // initialize all costs to INF
    for (i =0; i < GRID_WIDTH; i++) {
        for (j =0; j < GRID_HEIGHT; j++) {
            dNode[i][j]->cost = INT32_MAX;
        }
    }
    // setting cost of player loco to zero
    dNode[start->x][start->y]->cost = 0;

    heap_init(&h, cost_cmp, NULL);

    for (i =1; i < GRID_WIDTH; i++) {
        for( j =1; j < GRID_HEIGHT; j++) {
            dNode[i][j]->hn = heap_insert(&h, &dNode[i][j]);
        }
    }

    while (h.size > 0) {
        dijk_map_t *dn = (dijk_map_t *)heap_remove_min(&h);
        if (dn->cost == INT32_MAX) {
            break;
        }
    }
    //mark currnode as visited
    dn->visited = 1;

    for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++){
            int neighborX = dn->pos->x + dx;
            int neighborY = dn->pos->y + dy;

            if(neighborX >= 0 && neighborY <GRID_WIDTH &&
               neighborY >= 0 && neighborY <GRID_HEIGHT){
                dijk_map_t *neighbor = dNode[neighborX][neighborY];
                // ik this shit wrong, but need to fix other stuff before code compiles to this line
                int newCost = dn->cost + hiker_cost(*npc->tile);

                if(newCost < neighbor->cost) {
                    neighbor->cost = newCost;
                    heap_decrease_key_no_replace(&h, neighbor->hn);
                }
            }
        }
    }
}




void print_dijkstra_map(dijk_map_t *dNode[GRID_WIDTH][GRID_HEIGHT]) {
    int i, j;
    for (i = 0; i < GRID_WIDTH; i++) {
        for (j = 0; j < GRID_HEIGHT; j++) {
            if (dNode[i][j]->cost < 10) {
                printf(" %2d ", dNode[i][j]->cost % 100);
            } else {
                printf("%2d ", dNode[i][j]->cost % 100);
            }
        }
        printf("\n");
    }
}

void dijkstra_free(dijk_map_t dNode[GRID_WIDTH][GRID_HEIGHT]) {
    (void) pathTile;
    (void) tiles[TileCount];

    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            free(dNode[i][j].pos);
            free(dNode[i][j].prev);
        }
    }
}

