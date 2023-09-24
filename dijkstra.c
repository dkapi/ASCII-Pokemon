#include "heap.h"
#include "dijkstra.h"
#include <stdio.h>



static int32_t path_cmp(const void *key, const void *with) {
  return ((dijkstra_node_t *) key)->cost - ((dijkstra_node_t * )with)-> cost;
}

// static int32_t edge_penalty(int8_t x, int8_t y)
// {
//   return (x == 1 || y == 1 || x == GRID_WIDTH - 2 || y == GRID_HEIGHT - 2) ? 2 : 1;
// }

static int32_t isValid(int8_t x, int8_t y) {
    return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
}

void dijkstra_map(terrain_map_t *map, Location_t *start, 
                dijkstra_node_t *dNode[GRID_WIDTH][GRID_HEIGHT],
                dijkstra_node_t *dn)
{   
    int i, j;
    static uint32_t initialized = 0;
    heap_t h;

    if(!initialized){
        for (i =0; i < GRID_WIDTH;i++){
            for (j =0; j < GRID_HEIGHT; j++){
                dNode[i][j]->pos = malloc(sizeof(Location_t));
                dNode[i][j]->prev = malloc(sizeof(Location_t));
                dNode[i][j]->pos->x = i;
                dNode[i][j]->pos->y = j;
            }
        }
        initialized = 1;
    }

    for ( i = 0; i < GRID_WIDTH; i++){
        for ( j =0; j < GRID_HEIGHT; j++){
            dNode[i][j]->cost = __INT_MAX__;
            dNode[i][j]->visited =0;
            dNode[i][j]->prev->x = -1;
            dNode[i][j]->prev->y = -1;
            dNode[i][j]->hn = NULL;
        }
    }

    dNode[start->x][start->y]->cost = 0;

    heap_init(&h, path_cmp, NULL);

   dNode[start->x][start->y]->hn = heap_insert(&h,&dNode[start->x][start->y]);
   
    while(h.size >0){
        dn = (dijkstra_node_t *)heap_remove_min(&h);
        dn->visited = 1;

        int dr[] = { -1, -1, 0, 0};
        int dc[] = { 0, 0, -1, -1};

        for(i = 0; i < 4; i++){
            Location_t neighbor;
            neighbor.x = map->location.x + dr[i]; 
            neighbor.y = map->location.y + dc[i];

            if (neighbor.x <= 0 || neighbor.x >= GRID_WIDTH - 1 ||
                neighbor.y <= 0 || neighbor.y >= GRID_HEIGHT - 1) {
                continue;
            }
    

            if(isValid(neighbor.x,neighbor.y)){
                char terrain = map->grid[neighbor.x][neighbor.y];
                int terrainCost = 0;

                switch(terrain) {
                    case '#':
                        terrainCost = 10;
                        break;
                    case '.':
                        terrainCost = 10;
                        break;
                    case 'M':
                    case 'C':
                        terrainCost = 50;
                        break;
                    case ':':
                    case '^':
                        terrainCost = 15;
                        break;
                    default:
                        terrainCost = __INT_MAX__;
                        break;
                }

                int newCost = dn->cost + terrainCost;
                if(newCost < dNode[neighbor.x][neighbor.y]->cost) {
                    dNode[neighbor.x][neighbor.y]->cost = newCost;
                }
            }
        }
    }
}

void print_dijkstra_map(dijkstra_node_t *dNode[GRID_WIDTH][GRID_HEIGHT]) {
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

void dijkstra_free(dijkstra_node_t dNode[GRID_WIDTH][GRID_HEIGHT]) {
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            free(dNode[i][j].pos);
            free(dNode[i][j].prev);
        }
    }
}

