#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <termios.h>
#include <unistd.h>

#include "worldgen.h"
#include "voronoi.h"
#include "characters.h"
#include "dijkstra.h"
#include "heap.h"
#include "battle.h"

/* ------------------------------------------------------------------------- *
 * Terminal: raw mode so keys register instantly (no Enter required).        *
 * ------------------------------------------------------------------------- */
static struct termios saved_termios;

static void enable_raw_mode(void)
{
  struct termios raw;
  tcgetattr(STDIN_FILENO, &saved_termios);
  raw = saved_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;   /* 0.1 s read timeout */
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void restore_terminal(void)
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_termios);
}

enum { K_UP = 0x100, K_DOWN, K_LEFT, K_RIGHT };

/* Returns the key value, or 0 when no key was pressed within the timeout,
 * or -1 on end-of-input. */
static int read_key(void)
{
  unsigned char b, b2, b3;
  ssize_t n = read(STDIN_FILENO, &b, 1);

  if (n != 1) {
    if (n == 0) {
      return isatty(STDIN_FILENO) ? 0 : -1;
    }
    return 0;
  }
  if (b != '\033') {
    return b;
  }
  if (read(STDIN_FILENO, &b2, 1) != 1) return '\033';
  if (b2 != '[') return '\033';
  if (read(STDIN_FILENO, &b3, 1) != 1) return '\033';
  switch (b3) {
    case 'A': return K_UP;
    case 'B': return K_DOWN;
    case 'C': return K_RIGHT;
    case 'D': return K_LEFT;
    default:  return '\033';
  }
}

static int read_key_blocking(void)
{
  int k;
  while ((k = read_key()) == 0) {
    /* wait for the player */
  }
  return k;
}

static void wait_any_key(void)
{
  while (read_key() == 0) {
    /* spin until a key arrives */
  }
}

/* ------------------------------------------------------------------------- *
 * Rendering + map entry.                                                    *
 * ------------------------------------------------------------------------- */
void print_map(terrain_map_t *map)
{
  int i, j;

  for (i = 0; i < GRID_HEIGHT; i++) {
    for (j = 0; j < GRID_WIDTH; j++) {
      const char *color = "";
      char c;

      if (map->cmap[i][j]) {
        c = map->cmap[i][j]->ascii;
        color = (c == '@') ? WHITE : CYAN;
      } else {
        c = map->grid[i][j];
        switch (c) {
          case '%': color = MAGENTA; break;
          case '.': color = YELLOW;  break;
          case '~': color = BLUE;    break;
          case '^': color = GREEN;   break;
          case ':': color = GREEN;   break;
          case '#': color = BROWN;   break;
          case 'C': color = RED;     break;
          case 'M': color = LBLUE;   break;
          default:
            fprintf(stderr, "unhandled color case\n");
            break;
        }
      }
      printf("%s%c", color, c);
    }
    putchar('\n');
  }
  printf("%s", RESET);
}

static void draw_frame(terrain_map_t *g, Location_t loc, const char *status)
{
  printf("%s", CLEARSCREEN);
  print_map(g);
  printf("%scurrent location: (%d,%d)   [?=help]%s\n",
         WHITE, loc.x - 200, loc.y - 200, RESET);
  printf("%s$%d   %d potions   %d balls%s\n",
         WHITE, bag.money, bag.potions, bag.pokeballs, RESET);
  if (status[0]) {
    printf("%s%s%s\n", YELLOW, status, RESET);
  }
  fflush(stdout);
}

/* Reset the current map to a fresh PC + NPC population and rebuild the
 * hiker/rival distance maps for the new PC position. */
static void setup_map(terrain_map_t *g)
{
  clear_characters(g);
  place_pc(g);
  place_all_npcs(g);
  compute_dist_maps(g, &pc.location);
}

/* Load an already-generated map, or generate it, at world location `loc`. */
static terrain_map_t *load_map(terrain_map_t *(*worldMap)[WORLD_WIDTH],
                               Location_t loc, Gates_t gates)
{
  if (!worldMap[loc.x][loc.y]) {
    terrain_map_t *g = malloc(sizeof *g);
    assert(g);
    g->location = loc;
    generate_voronoi_map(g, gates);
    worldMap[loc.x][loc.y] = g;
  }
  return worldMap[loc.x][loc.y];
}

static void rebuild_turn_queue(heap_t *turn)
{
  heap_delete(turn);
  heap_init(turn, cmp_char_turn, NULL);

  for (int i = 0; i < NUM_NPCS; i++) {
    npc_list[i]->next_turn = 0;
    heap_insert(turn, npc_list[i]);
  }
  pc.next_turn = 0;
  heap_insert(turn, &pc);
}

/* Cross a border gate: update world coords + carry the exit gate across. */
static Location_t leave_map(int exit, Location_t loc, Gates_t *gates)
{
  switch (exit) {
    case PC_MOVE_EXIT_UP:    /* crossed top -> north (+y) */
      loc.y++;
      gates->bottom = (Location_t) { GRID_HEIGHT - 1, gates->top.y };
      gates->top = gates->left = gates->right = (Location_t) { 0, 0 };
      break;
    case PC_MOVE_EXIT_DOWN:  /* crossed bottom -> south (-y) */
      loc.y--;
      gates->top = (Location_t) { 0, gates->bottom.y };
      gates->bottom = gates->left = gates->right = (Location_t) { 0, 0 };
      break;
    case PC_MOVE_EXIT_LEFT:  /* crossed left -> west (-x) */
      loc.x--;
      gates->right = (Location_t) { gates->left.x, GRID_WIDTH - 1 };
      gates->top = gates->bottom = gates->left = (Location_t) { 0, 0 };
      break;
    case PC_MOVE_EXIT_RIGHT: /* crossed right -> east (+x) */
      loc.x++;
      gates->left = (Location_t) { gates->right.x, 0 };
      gates->top = gates->bottom = gates->right = (Location_t) { 0, 0 };
      break;
  }
  return loc;
}

/* ------------------------------------------------------------------------- *
 * Input.                                                                    *
 * ------------------------------------------------------------------------- */
static int read_int_prompt(const char *prompt)
{
  char buf[16];
  int i = 0;
  unsigned char c;

  printf("%s", prompt);
  fflush(stdout);

  for (;;) {
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n < 1) continue;
    if (c == '\n' || c == '\r') break;
    if ((c == 127 || c == 8) && i > 0) {
      i--;
      printf("\b \b");
      fflush(stdout);
      continue;
    }
    if (i < (int) sizeof(buf) - 1) {
      buf[i++] = (char) c;
      putchar(c);
      fflush(stdout);
    }
  }
  buf[i] = '\0';
  putchar('\n');
  return atoi(buf);
}

static void print_help(void)
{
  printf("%s", CLEARSCREEN);
  printf("========== ASCII-Pokemon : controls ==========\n\n");
  printf("  move          arrow keys, or n / s / e / w (one tile)\n");
  printf("  rest          . or any other key\n");
  printf("  fly           f  (then enter x and y, -200..200)\n");
  printf("  help          h or ?\n");
  printf("  quit          q  (or Ctrl-C)\n\n");
  printf("  time passes by move cost: path=10 grass=20 water=7\n");
  printf("  The world is 401 x 401 maps; the center is (0,0).\n\n");
  printf("  @ you  %% mountain  ^ tree  : grass  ~ water  # path\n");
  printf("  M mart  C center\n");
  printf("  NPCs: h hiker  r rival  p pacer  w wanderer\n");
  printf("        s sentry  e explorer  m swimmer\n");
  printf("  walk in tall grass ':' to find wild pokemon\n");
  printf("  walk into a trainer to battle\n");
  printf("  step on 'C' to heal, 'M' to visit the mart\n\n");
  printf("press any key to return...\n");
  fflush(stdout);
  wait_any_key();
}

/* ------------------------------------------------------------------------- *
 * Main game loop: a turn queue keyed by accumulated move cost.              *
 * ------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  int i, j;
  bool debug = false;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
      debug = true;
    }
  }

  srand(time(NULL));

  terrain_map_t *(*worldMap)[WORLD_WIDTH] =
    calloc(WORLD_HEIGHT, sizeof *worldMap);
  assert(worldMap);

  Location_t currLoc = { .x = 200, .y = 200 };
  Gates_t gates = { 0 };
  terrain_map_t *currGrid = load_map(worldMap, currLoc, gates);
  gates = currGrid->gates;
  setup_map(currGrid);

  if (debug) {
    printf("\n hiker cost map\n");
    print_dist_map(hiker_dist);
    printf("\nrival cost map\n");
    print_dist_map(rival_dist);
  }

  enable_raw_mode();

  pokemon_t pc_pokemon;
  starter_pick(&pc_pokemon, read_key_blocking);

  heap_t turn;
  heap_init(&turn, cmp_char_turn, NULL);
  rebuild_turn_queue(&turn);

  char status[96];
  bool shouldQuit = false;
  draw_frame(currGrid, currLoc, "");

  while (!shouldQuit) {
    struct character_s *cur = (struct character_s *) heap_remove_min(&turn);
    if (!cur) {
      break;
    }

    if (cur != &pc) {
      /* NPC's turn: act, then re-enter the queue at its new next_turn. */
      move_npc(currGrid, cur);
      heap_insert(&turn, cur);
      continue;
    }

    /* ----- PC's turn: render, then wait for the player. ----- */
    draw_frame(currGrid, currLoc, status);
    status[0] = '\0';

    int key = read_key_blocking();
    int dx = 0, dy = 0;
    bool move_requested = false;
    bool transitioned = false;

    switch (key) {
      case -1:
      case 'q':
      case 'Q':
        shouldQuit = true;
        break;

      case 'h':
      case '?':
        print_help();
        break;   /* help costs no turn */

      case 'f': {
        int x = read_int_prompt("fly to x [-200, 200]: ");
        int y = read_int_prompt("fly to y [-200, 200]: ");
        if (x >= -200 && x <= 200 && y >= -200 && y <= 200) {
          currLoc.x = x + 200;
          currLoc.y = y + 200;
          gates = (Gates_t) { 0 };
          currGrid = load_map(worldMap, currLoc, gates);
          gates = currGrid->gates;
          setup_map(currGrid);
          transitioned = true;
        } else {
          snprintf(status, sizeof status,
                   "you cant fly that far on your pidgey!");
        }
        break;
      }

      case 'n': case K_UP:    dx = -1; dy = 0;  move_requested = true; break;
      case 's': case K_DOWN:  dx =  1; dy = 0;  move_requested = true; break;
      case 'e': case K_RIGHT: dx =  0; dy = 1;  move_requested = true; break;
      case 'w': case K_LEFT:  dx =  0; dy = -1; move_requested = true; break;

      default:
        /* rest: no move, but the turn is consumed */
        pc.next_turn += REST_COST;
        break;
    }

    if (shouldQuit) {
      break;
    }

    if (move_requested) {
      int res = pc_try_move(currGrid, dx, dy);
      if (res == PC_MOVE_BLOCKED) {
        snprintf(status, sizeof status, "that way is blocked");
        pc.next_turn += REST_COST;
      } else if (res == PC_MOVE_BATTLE) {
        int tx = pc.location.x + dx, ty = pc.location.y + dy;
        struct character_s *foe = currGrid->cmap[tx][ty];
        if (foe) {
          int md = abs(currLoc.x - 200) + abs(currLoc.y - 200);
          trainer_battle(&pc_pokemon, read_key_blocking, foe, md);
        }
        pc.next_turn += REST_COST;
      } else if (res >= PC_MOVE_EXIT_UP) {
        currLoc = leave_map(res, currLoc, &gates);
        currGrid = load_map(worldMap, currLoc, gates);
        gates = currGrid->gates;
        setup_map(currGrid);
        transitioned = true;
      } else {
        /* Moved within the map: buildings, then tall-grass encounters. */
        char t = currGrid->grid[pc.location.x][pc.location.y];
        if (t == pokemonCenter.ascii) {
          pc_pokemon.cur_hp = pc_pokemon.hp;
          snprintf(status, sizeof status, "Nurse Joy healed your %s!",
                   species_name(pc_pokemon.species));
        } else if (t == pokeMart.ascii) {
          shop_mart(read_key_blocking);
        } else if (t == tallGrassTile.ascii &&
                   (rand() % 100) < ENCOUNTER_PROB) {
          int md = abs(currLoc.x - 200) + abs(currLoc.y - 200);
          wild_encounter(&pc_pokemon, read_key_blocking, md);
        }
      }
    }

    if (transitioned) {
      rebuild_turn_queue(&turn);
      continue;
    }

    heap_insert(&turn, &pc);
  }

  restore_terminal();
  printf("%s", CLEARSCREEN);
  printf("But how are you going to be the very best if you quit..\n");

  heap_delete(&turn);   /* free the remaining turn-queue nodes */

  for (i = 0; i < WORLD_HEIGHT; i++) {
    for (j = 0; j < WORLD_WIDTH; j++) {
      free(worldMap[i][j]);
    }
  }
  free(worldMap);

  return 0;
}