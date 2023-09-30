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

void dijkstra_map(terrain_map_t *map, Location_t *start, dijk_map_t *dNode[GRID_HEIGHT][GRID_WIDTH], dijk_map_t *dn, struct character_s *npc)
{   
    int i, j;
    static uint32_t initialized = 0;
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
                dNode[i][j]->pos.x = j;
                //dNode[i][j]->pos = newLoc;
                dNode[i][j]->visited =0;
                //dNode[i][j]->hn = NULL;
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

    heap_init(&h, cost_cmp, NULL);

    for (i =1; i < GRID_HEIGHT; i++) {
        for( j =1; j < GRID_WIDTH; j++) {
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
            int neighborX = dn->pos.x + dx;
            int neighborY = dn->pos.y + dy;

            if(neighborX >= 0 && neighborY <GRID_HEIGHT &&
               neighborY >= 0 && neighborY <GRID_WIDTH){
                dijk_map_t **neighbor = &dNode[neighborX][neighborY];
                // ik this shit wrong, but need to fix other stuff before code compiles to this line
                int newCost = dn->cost + npc->cost;

                if(newCost < (*neighbor)->cost) {
                    (*neighbor)->cost = newCost;
                    heap_decrease_key(&h, (*neighbor)->hn, NULL);
                }
            }
        }
    }
    // free dNode, idk if i need this here since i want to keep dNode values?
    for (i = 0; i < GRID_HEIGHT; i++) {
    for (j = 0; j < GRID_WIDTH; j++) {
        free(dNode[i][j]);
    }
}
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

// void dijkstra_free(dijk_map_t dNode[GRID_WIDTH][GRID_HEIGHT]) {
//     (void) pathTile;
//     (void) tiles[TileCount];

//     for (int i = 0; i < GRID_WIDTH; i++) {
//         for (int j = 0; j < GRID_HEIGHT; j++) {
//             free(dNode[i][j].pos.x);
//             free(dNode[i][j].prev);
//         }
//     }
// }

