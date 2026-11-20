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

/* Arrow keys arrive as ESC [ A/B/C/D.  Codes sit above 0xFF so they can't
 * collide with printable ASCII. */
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

static void wait_any_key(void)
{
  while (read_key() == 0) {
    /* spin until a key arrives */
  }
}

/* ------------------------------------------------------------------------- *
 * Rendering.                                                                *
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

/* Reset every map entry to a fresh PC + NPC population. */
static void setup_map(terrain_map_t *g)
{
  clear_characters(g);
  place_pc(g);
  place_all_npcs(g);
  compute_dist_maps(g, &pc.location);
}

/* ------------------------------------------------------------------------- *
 * Input handling.                                                           *
 * ------------------------------------------------------------------------- */

/* Move one world-map in direction 'n'/'s'/'e'/'w'.  Updates *loc in place
 * and returns true if the position changed (false = blocked by the edge). */
static bool move_char(Location_t *loc, Gates_t *gates, int dir)
{
  switch (dir) {
    case 'n':   /* north: +y */
      if (loc->y == 400) return false;
      loc->y++;
      gates->bottom = (Location_t) { GRID_HEIGHT - 1, gates->top.y };
      gates->top = gates->left = gates->right = (Location_t) { 0, 0 };
      return true;

    case 's':   /* south: -y */
      if (loc->y == 0) return false;
      loc->y--;
      gates->top = (Location_t) { 0, gates->bottom.y };
      gates->bottom = gates->left = gates->right = (Location_t) { 0, 0 };
      return true;

    case 'e':   /* east: +x */
      if (loc->x == 400) return false;
      loc->x++;
      gates->left = (Location_t) { gates->right.x, 0 };
      gates->top = gates->bottom = gates->right = (Location_t) { 0, 0 };
      return true;

    case 'w':   /* west: -x */
      if (loc->x == 0) return false;
      loc->x--;
      gates->right = (Location_t) { gates->left.x, GRID_WIDTH - 1 };
      gates->top = gates->bottom = gates->left = (Location_t) { 0, 0 };
      return true;

    default:
      return false;
  }
}

/* Read an integer from the terminal, echoing as the user types (raw mode
 * disables echo, so we do it manually). */
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
    if ((c == 127 || c == 8) && i > 0) {   /* backspace */
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
  printf("  move          arrow keys, or n / s / e / w\n");
  printf("  rest          any other key (NPCs still act)\n");
  printf("  fly           f  (then enter x and y, -200..200)\n");
  printf("  help          h or ?\n");
  printf("  quit          q  (or Ctrl-C)\n\n");
  printf("  The world is 401 x 401 maps; the center is (0,0).\n\n");
  printf("  @ you  %% mountain  ^ tree  : grass  ~ water  # path\n");
  printf("  M mart  C center\n");
  printf("  NPCs: h hiker  r rival  p pacer  w wanderer\n");
  printf("        s sentry  e explorer  m swimmer\n\n");
  printf("press any key to return...\n");
  fflush(stdout);
  wait_any_key();
}

/* ------------------------------------------------------------------------- *
 * Main.                                                                     *
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

  terrain_map_t *currGrid = malloc(sizeof *currGrid);
  assert(currGrid);

  Location_t currLoc = { .x = 200, .y = 200 };
  currGrid->location = currLoc;
  Gates_t gates = { 0 };
  generate_voronoi_map(currGrid, gates);
  worldMap[currLoc.x][currLoc.y] = currGrid;
  gates = currGrid->gates;

  setup_map(currGrid);

  if (debug) {
    printf("\n hiker cost map\n");
    print_dist_map(hiker_dist);
    printf("\nrival cost map\n");
    print_dist_map(rival_dist);
  }

  printf("%s", CLEARSCREEN);
  print_map(currGrid);
  printf("%scurrent location: (%d,%d)   [?=help]%s\n",
         WHITE, currLoc.x - 200, currLoc.y - 200, RESET);
  fflush(stdout);

  enable_raw_mode();

  char status[96];
  bool shouldQuit = false;
  while (!shouldQuit) {
    int key = read_key();
    bool moved = false;
    Location_t newLoc = currLoc;

    status[0] = '\0';

    switch (key) {
      case 0:
        continue;             /* no key yet */

      case -1:                /* stdin reached end-of-input */
        shouldQuit = true;
        break;

      case 'q':
      case 'Q':
        shouldQuit = true;
        break;

      case 'h':
      case '?':
        print_help();
        break;

      case 'f': {
        int x = read_int_prompt("fly to x [-200, 200]: ");
        int y = read_int_prompt("fly to y [-200, 200]: ");
        if (x >= -200 && x <= 200 && y >= -200 && y <= 200) {
          newLoc.x = x + 200;
          newLoc.y = y + 200;
          gates = (Gates_t) { 0 };
          moved = true;
        } else {
          snprintf(status, sizeof status,
                   "you cant fly that far on your pidgey!");
        }
        break;
      }

      case 'n': case K_UP:
        if (!move_char(&newLoc, &gates, 'n')) {
          snprintf(status, sizeof status,
                   "you've reached the border pokemon master!");
        } else {
          moved = true;
        }
        break;

      case 's': case K_DOWN:
        if (!move_char(&newLoc, &gates, 's')) {
          snprintf(status, sizeof status,
                   "you've reached the border pokemon master!");
        } else {
          moved = true;
        }
        break;

      case 'e': case K_RIGHT:
        if (!move_char(&newLoc, &gates, 'e')) {
          snprintf(status, sizeof status,
                   "you've reached the border pokemon master!");
        } else {
          moved = true;
        }
        break;

      case 'w': case K_LEFT:
        if (!move_char(&newLoc, &gates, 'w')) {
          snprintf(status, sizeof status,
                   "you've reached the border pokemon master!");
        } else {
          moved = true;
        }
        break;

      default:
        break;
    }

    if (shouldQuit) {
      break;
    }

    if (moved) {
      if (!worldMap[newLoc.x][newLoc.y]) {
        terrain_map_t *newGrid = malloc(sizeof *newGrid);
        assert(newGrid);
        newGrid->location = newLoc;
        generate_voronoi_map(newGrid, gates);
        worldMap[newLoc.x][newLoc.y] = newGrid;
      }
      currGrid = worldMap[newLoc.x][newLoc.y];
      setup_map(currGrid);
      currLoc = newLoc;
      gates = currGrid->gates;
    }

    /* NPCs act every turn, whether the PC moved or rested. */
    move_npcs(currGrid);

    printf("%s", CLEARSCREEN);
    print_map(currGrid);
    printf("%scurrent location: (%d,%d)   [?=help]%s\n",
           WHITE, currLoc.x - 200, currLoc.y - 200, RESET);
    if (status[0]) {
      printf("%s%s%s\n", YELLOW, status, RESET);
    }
    fflush(stdout);
  }

  restore_terminal();
  printf("%s", CLEARSCREEN);
  printf("But how are you going to be the very best if you quit..\n");

  for (i = 0; i < WORLD_HEIGHT; i++) {
    for (j = 0; j < WORLD_WIDTH; j++) {
      free(worldMap[i][j]);
    }
  }
  free(worldMap);

  return 0;
}