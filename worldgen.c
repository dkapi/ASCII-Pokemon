#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <ncurses.h>
#include "worldgen.h"
#include "voronoi.h"
#include "characters.h"
#include "dijkstra.h"


terrain_map_t* worldMap[WORLD_HEIGHT][WORLD_WIDTH];

struct tile_s edgeTile = 
{
    .ascii = '%',
    .name = "edgeTile",
    .tile_idx = EdgeTile,
};

void print_map(terrain_map_t *map) // TODO: take grid not map
{
    for(int i = 0; i < GRID_HEIGHT; i++) {
        for(int j = 0; j < GRID_WIDTH; j++) {
            char *color = "";
            switch (map->grid[i][j]->ascii) {
                case '%':
                    color = MAGENTA;
                    break;
                case '.':
                    color = YELLOW;
                    break;
                case '~':
                    color = BLUE;
                    break;
                case '^':
                    color = GREEN;
                    break;
                case ':':
                    color = GREEN;
                    break;
                case '#':
                    color = BROWN;
                    break;
                case 'C':
                    color = RED;
                    break;
                case 'M':
                    color = LBLUE;
                    break;
                case '@': 
                    color = WHITE;
                    break;
                case 'h':
                    color = WHITE;
                    break;
                case 'r':
                    color = WHITE;
                    break;
                case 'p':
                    color = WHITE;
                    break;
                case 's':
                    color = WHITE;
                    break;
                case 'w':
                    color = WHITE;
                    break;
                case 'e':
                    color = WHITE;
                    break;
                default:
                    // unreachable
                    fprintf(stderr, "unhandled color case %c\n", map->grid[i][j]->ascii);
                    break;
            }
            printf("%s%c", color, map->grid[i][j]->ascii);
        }
        putchar('\n');
    }
}

void null_world_map()
{
    for(int i = 0; i < WORLD_HEIGHT; ++i) {
        for(int j = 0; j < WORLD_WIDTH; ++j) {
            worldMap[i][j] = NULL;
        }
    }
}

void free_world_map()
{
    for(int i =0; i < WORLD_HEIGHT; i++) {
        for(int j =0; j < WORLD_WIDTH; j++) {
            if(worldMap[i][j]){
            free(worldMap[i][j]);
            worldMap[i][j] = NULL;
            }
        }
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
        if(loc.y == 400) { printf("you've reached the border pokemon master!\n");
            break;
        }
            loc.y += loc.y >= 0 ? 1 : 0;
            gates->bottom = (Location_t) { .x = gates->bottom.x, .y = gates->top.y};
            gates->top = (Location_t){ .x = 0, .y = 0 };
            gates->left = (Location_t){ .x = 0, .y = 0 };
            gates->right = (Location_t){ .x = 0, .y = 0 };
            break; 
        case 's':
        if (loc.y == 0) {  printf("you've reached the border pokemon master!\n");
             break;
        }
            loc.y += loc.y < 401 ? -1 : 0;
            gates->top = (Location_t){ .x = gates->top.x, .y = gates->bottom.y };
            gates->bottom = (Location_t){ .x = 0, .y = 0 };
            gates->left = (Location_t){ .x = 0, .y = 0 };
            gates->right = (Location_t){ .x = 0, .y = 0 };
            break;
        case 'e':
        if(loc.x == 400) { printf("you've reached the border pokemon master!\n");
            break;
        }
            loc.x += loc.x >= 0 ? 1 : 0;
            gates->left = (Location_t){ .x = gates->right.x, .y = gates->left.y };
            gates->bottom = (Location_t){ .x = 0, .y = 0 };
            gates->top = (Location_t){ .x = 0, .y = 0 };
            gates->right = (Location_t){ .x = 0, .y = 0 };
            break;
        case 'w':
        if(loc.x == 0) { printf("you've reached the border pokemon master!\n");
            break;
        }
            loc.x += loc.x < 401 ? -1 : 0;
            gates->right = (Location_t){ .x = gates->left.x, .y = gates->right.y };
            gates->bottom = (Location_t){ .x = 0, .y = 0 };
            gates->top = (Location_t){ .x = 0, .y = 0 };
            gates->left = (Location_t){ .x = 0, .y = 0 };
            break;
       case 'f':
            int x,y;
            if(sscanf(buf, "f %d %d", &x, &y) == 2){
                if((-200 <= x && x <= 200) && (-200 <= y && y <= 200)) {
                    loc.x = x + 200;
                    loc.y = y + 200;
                    *gates = (Gates_t){0};
                }
                else {
                    printf("you cant fly that far on your pidgey!\n");
                }
            }
            break;
        case 'q':
            *quit = true;
            printf("But how are you going to be the very best if you quit..\n");
            break;
    }
    return loc;
}

static int32_t npc_cost_cmp(const void *key, const void *with) {
    struct character_s * k = (struct character_s*)key;
    struct character_s * w = (struct character_s*)with;

    uint32_t cost_k = k->travelCost;
    uint32_t cost_w = w->travelCost;

    return cost_k - cost_w;
}


void npc_movement_loop(terrain_map_t *currGrid, struct character_s npc_arr[])
{
    //todo pass in as param 
    int npc_arr_size = 5;
    int cardinalDirForPacer = rand() % 2;
    struct character_s *currNpc;
    int dx, dy, neighborX, neighborY;
    heap_t h;
    
    //this is main loop for npc movement, currently only way to exit is Ctrl + C
    for(;;){
        heap_init(&h, npc_cost_cmp, NULL);
        //enqueue all npcs
        for(int i =0; i < npc_arr_size; i++){
            heap_insert(&h, &npc_arr[i]);
        }

        //switch case to handle all npcs, keep hiker and rival as default.

        //pull out min in heap
        currNpc = heap_remove_min(&h);   
        //reset currenctCost for neighborloop 
        Location_t cheapestNeighbor = {.x = -1, .y = -1};

        switch(currNpc->self.ascii){
            case('p'):
                // Pacer NPC logic
                int nextX;
                int nextY;
                switch(cardinalDirForPacer) {
                    case(0):
                        nextX = currNpc->location.x + 0;
                        nextY = currNpc->location.y + pacer.direction;
                        break;
                    case(1):
                        nextX = currNpc->location.x + pacer.direction;
                        nextY = currNpc->location.y + 0;
                        break;
                    default:
                        // unreachable
                        fprintf(stderr, "error in picking pacer direction being S-N or E-W");
                }
                    
                // Check if the tile in front of the pacer NPC is impassable
                if (currNpc->cost(*currGrid->grid[nextX][nextY]) == INT32_MAX) {
                    // If impassable, change direction by turning around
                    pacer.direction = -pacer.direction;
                } else {
                    // Move the pacer NPC one tile
                    currNpc->travelCost += currNpc->cost(*currGrid->grid[nextX][nextY]);

                    currGrid->grid[currNpc->location.x][currNpc->location.y] = currNpc->standing;
                    currNpc->standing = currGrid->grid[nextX][nextY];
                    currNpc->location.x = nextX;
                    currNpc->location.y = nextY;
                    currGrid->grid[nextX][nextY] = &currNpc->self; // Update the new tile
                }
                break;
            //  case('s'):
            //     break;
             case('w'):
             // Wanderer NPC logic
                int dWanderX, dWanderY;

                // Generate a random direction (between -1, 0, 1) for both x and y
                dWanderX = rand() % 3 - 1;
                dWanderY = rand() % 3 - 1;

                // Check if the tile in the randomly chosen direction is passable
                int nextWanderX = currNpc->location.x + dWanderX;
                int nextWanderY = currNpc->location.y + dWanderY;

                if (currNpc->cost(*currGrid->grid[nextWanderX][nextWanderY]) != INT32_MAX) {
                    if(currGrid->grid[nextWanderX][nextWanderY] == wanderer.standing) {
                    // Move the wanderer NPC one tile
                    currNpc->travelCost += currNpc->cost(*currGrid->grid[nextWanderX][nextWanderY]);

                    currGrid->grid[currNpc->location.x][currNpc->location.y] = currNpc->standing;
                    currNpc->standing = currGrid->grid[nextWanderX][nextWanderY];
                    currNpc->location.x = nextWanderX;
                    currNpc->location.y = nextWanderY;
                    currGrid->grid[nextWanderX][nextWanderY] = &currNpc->self; // Update the new tile
                    }
                }
                break;
             case('e'):
             // Explorer NPC logic
                int dExplorX, dExplorY;

                // Generate a random direction (between -1, 0, 1) for both x and y
                dExplorX = rand() % 3 - 1;
                dExplorY = rand() % 3 - 1;

                // Check if the tile in the randomly chosen direction is passable
                int nextExplorX = currNpc->location.x + dExplorX;
                int nextExplorY = currNpc->location.y + dExplorY;

                if (currNpc->cost(*currGrid->grid[nextExplorX][nextExplorY]) != INT32_MAX) {
                    // Move the wanderer NPC one tile
                    currNpc->travelCost += currNpc->cost(*currGrid->grid[nextExplorX][nextExplorY]);

                    currGrid->grid[currNpc->location.x][currNpc->location.y] = currNpc->standing;
                    currNpc->standing = currGrid->grid[nextExplorX][nextExplorY];
                    currNpc->location.x = nextExplorX;
                    currNpc->location.y = nextExplorY;
                    currGrid->grid[nextExplorX][nextExplorY] = &currNpc->self; // Update the new tile
                }
                break;
            default:
                // loop to find cheapest neighbor
                for(dx = -1; dx <= 1; dx++) {
                    for(dy = -1; dy <= 1; dy++){

                        if(neighborX < 0 && neighborX >GRID_HEIGHT && neighborY < 0 && neighborY >GRID_WIDTH){
                            continue;
                        }
                        neighborX = currNpc->location.x + dx;
                        neighborY = currNpc->location.y + dy;
                        //TODO : <= should be < for a more straigh path to the pc, but because cost map does not have infinite border, it will mess up
                        if (currNpc->costMap[neighborX][neighborY].cost <= currNpc->costMap[cheapestNeighbor.x + currNpc->location.x][cheapestNeighbor.y + currNpc->location.y].cost){
                            cheapestNeighbor.x = dx;
                            cheapestNeighbor.y = dy;
                        }
                    }
                }
                
                currNpc->travelCost = currNpc->travelCost + currNpc->cost(*currGrid->grid[cheapestNeighbor.x + currNpc->location.x][cheapestNeighbor.y + currNpc->location.y]); 

                neighborX = currNpc->location.x + cheapestNeighbor.x;
                neighborY = currNpc->location.y + cheapestNeighbor.y;

                if(currGrid->grid[neighborX][neighborY]->ascii == currGrid->grid[pc.location.x][pc.location.y]->ascii){
                    continue;
                }
                if(currGrid->grid[neighborX][neighborY]->ascii == 'h' || currGrid->grid[neighborX][neighborY]->ascii == 'r' || currGrid->grid[neighborX][neighborY]->ascii == 'p' ||
                   currGrid->grid[neighborX][neighborY]->ascii == 'w' || currGrid->grid[neighborX][neighborY]->ascii == 'e' || currGrid->grid[neighborX][neighborY]->ascii == 's'){
                    continue;
                }

                currGrid->grid[currNpc->location.x][currNpc->location.y] = currNpc->standing;
                currNpc->standing = currGrid->grid[neighborX][neighborY];
                currNpc->location.x = neighborX;
                currNpc->location.y = neighborY;
                currGrid->grid[neighborX][neighborY] = &currNpc->self;
        }
        heap_delete(&h);
        usleep(250000);
        print_map(currGrid);
        printf("\n");
    }
}

void place_all_npcs(terrain_map_t *currGrid)
{
    place_npc(currGrid, &hiker);
    //place_npc(currGrid, &rival);
    place_npc(currGrid, &pacer);
    place_npc(currGrid, &wanderer);
    place_npc(currGrid, &explorer);
    place_npc(currGrid, &sentry);
}

int main(int argc, char *argv[])
{
    (void)tiles[TileCount];
    (void)pathTile;
    srand(time(NULL));
    null_world_map();


    terrain_map_t* currGrid = (terrain_map_t*)malloc(sizeof(terrain_map_t));
    Location_t currLoc = { .x = 200, .y = 200 };
    currGrid->location = currLoc;
    Gates_t gates = {0};
    generate_voronoi_map(currGrid, gates);
    place_pc(currGrid);
    place_all_npcs(currGrid);
    worldMap[currGrid->location.x][currGrid->location.y] = currGrid;
    gates = currGrid->gates;
   // print_map(worldMap[currLoc.x][currLoc.y]);


    // Ncurses initialization code
    initscr();
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);

    for(int i = 0; i < GRID_HEIGHT; i++) {
        for(int j = 0; j < GRID_WIDTH; j++) {
            mvaddch(i,j, currGrid->grid[i][j]->ascii);
        }
    }
    refresh();

    //variable to store the ascii int value getch will return
    int ch;
    int quit = 1;
    while(quit != 0){
        ch = getch();

        switch(ch) {
            case '7':
            case 'y': 
                if(currGrid->grid[pc.location.x-1][pc.location.y-1]->ascii != '%' || currGrid->grid[pc.location.x-1][pc.location.y-1]->ascii != '^' ||
                   currGrid->grid[pc.location.x-1][pc.location.y-1]->ascii != '~' || currGrid->grid[pc.location.x-1][pc.location.y-1]->ascii != 'h' || 
                   currGrid->grid[pc.location.x-1][pc.location.y-1]->ascii != 'r' || currGrid->grid[pc.location.x-1][pc.location.y-1]->ascii != 's' ||
                   currGrid->grid[pc.location.x-1][pc.location.y-1]->ascii != 'e' || currGrid->grid[pc.location.x-1][pc.location.y-1]->ascii != 'w'){
                    
                    currGrid->grid[pc.location.x][pc.location.y] = pc.standing;
                    pc.standing = currGrid->grid[pc.location.x-1][pc.location.y-1];
                    pc.location.x = pc.location.x-1;
                    pc.location.y = pc.location.y-1;
                    currGrid->grid[pc.location.x][pc.location.y] = &pc.self;
                    clear();

                   }
                break;
            case '8':
            case 'k':
                //code to move pc one cell up
                if(currGrid->grid[pc.location.x-1][pc.location.y]->ascii != '%' || currGrid->grid[pc.location.x-1][pc.location.y]->ascii != '^' ||
                   currGrid->grid[pc.location.x-1][pc.location.y]->ascii != '~' || currGrid->grid[pc.location.x-1][pc.location.y]->ascii != 'h' || 
                   currGrid->grid[pc.location.x-1][pc.location.y]->ascii != 'r' || currGrid->grid[pc.location.x-1][pc.location.y]->ascii != 's' ||
                   currGrid->grid[pc.location.x-1][pc.location.y]->ascii != 'e' || currGrid->grid[pc.location.x-1][pc.location.y]->ascii != 'w'){
                    
                    currGrid->grid[pc.location.x][pc.location.y] = pc.standing;
                    pc.standing = currGrid->grid[pc.location.x-1][pc.location.y];
                    pc.location.x = pc.location.x-1;
                    pc.location.y = pc.location.y;
                    currGrid->grid[pc.location.x][pc.location.y] = &pc.self;
                    clear();
                   
                   }
                break;
            case '9':
            case 'u':
                //code to move pc one cel to the upper right
                if(currGrid->grid[pc.location.x-1][pc.location.y+1]->ascii != '%' || currGrid->grid[pc.location.x-1][pc.location.y+1]->ascii != '^' ||
                   currGrid->grid[pc.location.x-1][pc.location.y+1]->ascii != '~' || currGrid->grid[pc.location.x-1][pc.location.y+1]->ascii != 'h' || 
                   currGrid->grid[pc.location.x-1][pc.location.y+1]->ascii != 'r' || currGrid->grid[pc.location.x-1][pc.location.y+1]->ascii != 's' ||
                   currGrid->grid[pc.location.x-1][pc.location.y+1]->ascii != 'e' || currGrid->grid[pc.location.x-1][pc.location.y+1]->ascii != 'w'){
                    
                    currGrid->grid[pc.location.x][pc.location.y] = pc.standing;
                    pc.standing = currGrid->grid[pc.location.x-1][pc.location.y+1];
                    pc.location.x = pc.location.x-1;
                    pc.location.y = pc.location.y+1;
                    currGrid->grid[pc.location.x][pc.location.y] = &pc.self;
                    clear();
                   }
                break;
            case '6':
            case 'l':
                //code to move pc one cell to the right
                if(currGrid->grid[pc.location.x][pc.location.y+1]->ascii != '%' || currGrid->grid[pc.location.x][pc.location.y+1]->ascii != '^' ||
                   currGrid->grid[pc.location.x][pc.location.y+1]->ascii != '~' || currGrid->grid[pc.location.x][pc.location.y+1]->ascii != 'h' || 
                   currGrid->grid[pc.location.x][pc.location.y+1]->ascii != 'r' || currGrid->grid[pc.location.x][pc.location.y+1]->ascii != 's' ||
                   currGrid->grid[pc.location.x][pc.location.y+1]->ascii != 'e' || currGrid->grid[pc.location.x][pc.location.y+1]->ascii != 'w'){
                    
                    currGrid->grid[pc.location.x][pc.location.y] = pc.standing;
                    pc.standing = currGrid->grid[pc.location.x][pc.location.y+1];
                    pc.location.x = pc.location.x;
                    pc.location.y = pc.location.y+1;
                    currGrid->grid[pc.location.x][pc.location.y] = &pc.self;
                    clear();
                   }
                break;
            case '3':
            case 'n':
                //code to move pc one cell to lower right
                if(currGrid->grid[pc.location.x+1][pc.location.y+1]->ascii != '%' || currGrid->grid[pc.location.x+1][pc.location.y+1]->ascii != '^' ||
                   currGrid->grid[pc.location.x+1][pc.location.y+1]->ascii != '~' || currGrid->grid[pc.location.x+1][pc.location.y+1]->ascii != 'h' || 
                   currGrid->grid[pc.location.x+1][pc.location.y+1]->ascii != 'r' || currGrid->grid[pc.location.x+1][pc.location.y+1]->ascii != 's' ||
                   currGrid->grid[pc.location.x+1][pc.location.y+1]->ascii != 'e' || currGrid->grid[pc.location.x+1][pc.location.y+1]->ascii != 'w'){
                    
                    currGrid->grid[pc.location.x][pc.location.y] = pc.standing;
                    pc.standing = currGrid->grid[pc.location.x+1][pc.location.y+1];
                    pc.location.x = pc.location.x+1;
                    pc.location.y = pc.location.y+1;
                    currGrid->grid[pc.location.x][pc.location.y] = &pc.self;
                    clear();
                   }
                break;
            case '2':
            case 'j':
                //code to move pc cone cell down
                if(currGrid->grid[pc.location.x+1][pc.location.y]->ascii != '%' || currGrid->grid[pc.location.x+1][pc.location.y]->ascii != '^' ||
                   currGrid->grid[pc.location.x+1][pc.location.y]->ascii != '~' || currGrid->grid[pc.location.x+1][pc.location.y]->ascii != 'h' || 
                   currGrid->grid[pc.location.x+1][pc.location.y]->ascii != 'r' || currGrid->grid[pc.location.x+1][pc.location.y]->ascii != 's' ||
                   currGrid->grid[pc.location.x+1][pc.location.y]->ascii != 'e' || currGrid->grid[pc.location.x+1][pc.location.y]->ascii != 'w'){
                    
                    currGrid->grid[pc.location.x][pc.location.y] = pc.standing;
                    pc.standing = currGrid->grid[pc.location.x+1][pc.location.y];
                    pc.location.x = pc.location.x+1;
                    pc.location.y = pc.location.y;
                    currGrid->grid[pc.location.x][pc.location.y] = &pc.self;
                    clear();
                   }
                break;
            case '1':
            case 'b':
                //code to move pc one cell to lower left
                if(currGrid->grid[pc.location.x+1][pc.location.y-1]->ascii != '%' || currGrid->grid[pc.location.x+1][pc.location.y-1]->ascii != '^' ||
                   currGrid->grid[pc.location.x+1][pc.location.y-1]->ascii != '~' || currGrid->grid[pc.location.x+1][pc.location.y-1]->ascii != 'h' || 
                   currGrid->grid[pc.location.x+1][pc.location.y-1]->ascii != 'r' || currGrid->grid[pc.location.x+1][pc.location.y-1]->ascii != 's' ||
                   currGrid->grid[pc.location.x+1][pc.location.y-1]->ascii != 'e' || currGrid->grid[pc.location.x+1][pc.location.y-1]->ascii != 'w'){
                    
                    currGrid->grid[pc.location.x][pc.location.y] = pc.standing;
                    pc.standing = currGrid->grid[pc.location.x+1][pc.location.y-1];
                    pc.location.x = pc.location.x+1;
                    pc.location.y = pc.location.y-1;
                    currGrid->grid[pc.location.x][pc.location.y] = &pc.self;
                    clear();
                   }
                break;
            case '4':
            case 'h':
                //code to move pc one cell to left
                if(currGrid->grid[pc.location.x][pc.location.y-1]->ascii != '%' || currGrid->grid[pc.location.x][pc.location.y-1]->ascii != '^' ||
                   currGrid->grid[pc.location.x][pc.location.y-1]->ascii != '~' || currGrid->grid[pc.location.x][pc.location.y-1]->ascii != 'h' || 
                   currGrid->grid[pc.location.x][pc.location.y-1]->ascii != 'r' || currGrid->grid[pc.location.x][pc.location.y-1]->ascii != 's' ||
                   currGrid->grid[pc.location.x][pc.location.y-1]->ascii != 'e' || currGrid->grid[pc.location.x][pc.location.y-1]->ascii != 'w'){
                    
                    currGrid->grid[pc.location.x][pc.location.y] = pc.standing;
                    pc.standing = currGrid->grid[pc.location.x][pc.location.y-1];
                    pc.location.x = pc.location.x;
                    pc.location.y = pc.location.y-1;
                    currGrid->grid[pc.location.x][pc.location.y] = &pc.self;
                    clear();
                   }
                break;
            case '>':
                if(pc.standing->ascii == 'C' || pc.standing->ascii == 'M'){
                    clear();
                    mvaddstr(0,1,"welcome to the PokeMart/Center, this will be implimented later :)");
                    while(ch != '<'){
                        continue;
                    }
                }
                break;
            case '5':
            case ' ':
            case '.':
                //code for PC to rest a turn, meaning we dont move but rest of gamemap updates if needed
                break;
            case 't':
                //code to bring up list of trainers/npc
                break;
            case KEY_UP:
                // if trainer list is to large to fit on screen size of 80x24 then this will scroll up the list
                break;
            case KEY_DOWN:
                // if trainer list is to large to fit on screen size of 80x24 then this will scroll down the list
                break;
            case 27:
                // exits the trainer list thing
                break;
            case 'q':
            case 'Q':
                quit = 0;
                break;
            default:
                // whatever key was typed is not in use, dosent do anything
                break;
        } 
            for(int i = 0; i < GRID_HEIGHT; i++) {
                for(int j = 0; j < GRID_WIDTH; j++) {
                    mvaddch(i,j, currGrid->grid[i][j]->ascii);
                }
            }
            refresh();
    }

    endwin();

    // printf("%scurrent location: (%d,%d) movement input: ",WHITE,currLoc.x - 200,currLoc.y - 200 );

    // // create dijkstra cost maps for npc's that need em
    // dijkstra_map(currGrid, &pc.location, hiker.costMap, &hiker);
    // //dijkstra_map(currGrid, &pc.location, rival.costMap, &rival);
    // //print_dijkstra_map(hiker.costMap);
    // //print_dijkstra_map(rival.costMap);
    // // array to hold all the npc's that will spawn per Grid decided on the numTrainers flag, or default
    // struct character_s npc_arr[] = {hiker, /* rival, */ pacer, wanderer, explorer};
    // // npc movement.
    //  npc_movement_loop(currGrid, npc_arr);



    
    // this is input movement for worldMap traversal using n, s ,e ,w

    // char userInput[32];
    // bool shouldQuit = false;
    // while(!shouldQuit) {
    //     size_t input_n = sizeof(userInput)/sizeof(userInput[0]);
    //     get_input(input_n, &userInput[0]);
    //     Location_t newLoc = handle_input(input_n, &userInput[0], currLoc, &gates, &shouldQuit);
    //     if((currLoc.x != newLoc.x || currLoc.y != newLoc.y) && worldMap[newLoc.x][newLoc.y] == NULL) {
    //         terrain_map_t* newGrid = (terrain_map_t*)malloc(sizeof(terrain_map_t));
    //         newGrid->location = newLoc;
    //         generate_voronoi_map(newGrid, gates);
    //         worldMap[newGrid->location.x][newGrid->location.y] = newGrid;
    //         currGrid = newGrid;
    //     }
    //     print_map(worldMap[newLoc.x][newLoc.y]);
    //     printf("%scurrent location: (%d,%d), movment input: %c \n", WHITE,
    //             (newLoc.x < 0 ? newLoc.x +200 : newLoc.x -200),
    //             (newLoc.y < 0 ? newLoc.y +200 : newLoc.y -200),
    //             userInput[0]);
    //     currLoc = newLoc;
    //     gates = currGrid->gates;
    //}


    free_world_map();

    return 0;
}
