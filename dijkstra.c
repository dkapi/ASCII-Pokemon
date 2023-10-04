#include "heap.h"
#include "dijkstra.h"
#include "characters.h"
#include <stdio.h>

static int32_t character_cost_cmp(const void *key, const void *with) {
    dijk_map_t * k = (dijk_map_t*)key;
    dijk_map_t * w = (dijk_map_t*)with;

    uint32_t cost_k = k->cost;
    uint32_t cost_w = w->cost;

    return cost_k - cost_w;
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
        case '@':
        // for now this is pathtile because
        // the PC is stationary on path
            return &pathTile;
        default:
        // unreachable
        fprintf(stderr, "something went wrong in char_to_tile_s func");
        return &pathTile;
    } 
}

void dijkstra_map(terrain_map_t *map, Location_t *start, dijk_map_t dNode[GRID_HEIGHT][GRID_WIDTH], struct character_s *npc)
{   
    int i = 0; 
    int j = 0;
    static uint32_t initialized = 0;
    dijk_map_t *dn;
    heap_t h;

    //initialize the dNode to simulate given grid size, and set stuff
    if (initialized < 2) {
        for (i = 0; i < GRID_HEIGHT; i++) {
            for (j = 0; j < GRID_WIDTH; j++) {
                dNode[i][j].pos.x = i;
                dNode[i][j].pos.y = j;
                dNode[i][j].tile = char_to_tile_s(map->grid[i][j]);
            }
        }
        initialized++;
    }
    // initialize all costs to INF
    for (i =0; i < GRID_HEIGHT; i++) {
        for (j =0; j < GRID_WIDTH; j++) {
            dNode[i][j].cost = INT32_MAX;
        }
    }
    // setting cost of player loco to zero
    dNode[start->x][start->y].cost = 0;

   
    heap_init(&h, character_cost_cmp, NULL);

    for (i =0; i < GRID_HEIGHT; i++) {
        for( j =0; j < GRID_WIDTH; j++) {
            dNode[i][j].npc = npc;
            if (dNode[i][j].npc->cost(*dNode[i][j].tile) != INT32_MAX) {
                dNode[i][j].hn = heap_insert(&h, &dNode[i][j]);
            } else {
                dNode[i][j].hn = NULL;
            }
        }
    }
    while ((dn = heap_remove_min(&h))) {
        
    //mark currnode as visited
        dn->hn = NULL;
        int dx= 0, dy= 0, neighborX =0, neighborY = 0;
        uint32_t newCost = 0;
        int currCost = 0;
        dijk_map_t *neighbor;
        
        for(dx = -1; dx <= 1; dx++) {
            for(dy = -1; dy <= 1; dy++){
                neighborX = dn->pos.x + dx;
                neighborY = dn->pos.y + dy;

                if(neighborX >= 0 && neighborX <GRID_HEIGHT &&
                    neighborY >= 0 && neighborY <GRID_WIDTH){

                    neighbor = &dNode[neighborX][neighborY];
                    currCost = neighbor->npc->cost(*neighbor->tile);
                    
                    if(neighbor->hn){
                        newCost = dn->cost + npc->cost(*dn->tile);


                        if(newCost < neighbor->cost) {
                            neighbor->cost = newCost;
                            heap_decrease_key_no_replace(&h, neighbor->hn);
                        }
                    }
                }
            }
        }
    }
    heap_delete(&h);
}

void print_dijkstra_map(dijk_map_t dNode[GRID_HEIGHT][GRID_WIDTH]) {
    int i, j;
    for (i = 0; i < GRID_HEIGHT; i++) {
        for (j = 0; j < GRID_WIDTH; j++) {
            if (dNode[i][j].cost ==INT32_MAX) {
                printf("  ");
            } else {
                printf("%02d ", dNode[i][j].cost % 100);
            }
        }
        printf("\n");
    }
    (void) pathTile;
    (void) tiles[TileCount];
}



