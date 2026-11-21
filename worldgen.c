#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>

#include "worldgen.h"
#include "voronoi.h"
#include "characters.h"
#include "dijkstra.h"
#include "heap.h"
#include "battle.h"
#include "msg.h"

/* ------------------------------------------------------------------------- *
 * ncurses initialization + color pairs.                                     *
 * ------------------------------------------------------------------------- */
enum {
  CP_WHITE = 1, CP_CYAN, CP_MAGENTA, CP_YELLOW,
  CP_BLUE, CP_GREEN, CP_BROWN, CP_RED, CP_LBLUE,
};

static void init_curses(void)
{
  initscr();
  cbreak();                  /* no line buffering, but Ctrl-C still works */
  noecho();
  keypad(stdscr, TRUE);      /* arrow keys */
  curs_set(0);               /* hide cursor */
  start_color();
  init_pair(CP_WHITE,   COLOR_WHITE,   COLOR_BLACK);
  init_pair(CP_CYAN,    COLOR_CYAN,    COLOR_BLACK);
  init_pair(CP_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(CP_YELLOW,  COLOR_YELLOW,  COLOR_BLACK);
  init_pair(CP_BLUE,    COLOR_BLUE,    COLOR_BLACK);
  init_pair(CP_GREEN,   COLOR_GREEN,   COLOR_BLACK);
  init_pair(CP_BROWN,   COLOR_YELLOW,  COLOR_BLACK);
  init_pair(CP_RED,     COLOR_RED,     COLOR_BLACK);
  init_pair(CP_LBLUE,   COLOR_CYAN,    COLOR_BLACK);
}

static int read_key_blocking(void)
{
  return getch();
}

static void wait_any_key(void)
{
  getch();
}

/* ------------------------------------------------------------------------- *
 * Rendering.                                                                *
 * ------------------------------------------------------------------------- */
static int tile_pair(char c)
{
  switch (c) {
    case '%': return CP_MAGENTA;
    case '.': return CP_YELLOW;
    case '~': return CP_BLUE;
    case '^': return CP_GREEN;
    case ':': return CP_GREEN;
    case '#': return CP_BROWN;
    case 'C': return CP_RED;
    case 'M': return CP_LBLUE;
    default:  return CP_WHITE;
  }
}

void print_map(terrain_map_t *map)
{
  int i, j;

  for (i = 0; i < GRID_HEIGHT; i++) {
    for (j = 0; j < GRID_WIDTH; j++) {
      char c;
      int pair;

      if (map->cmap[i][j]) {
        c = map->cmap[i][j]->ascii;
        pair = (c == '@') ? CP_WHITE : CP_CYAN;
      } else {
        c = map->grid[i][j];
        pair = tile_pair(c);
      }

      attron(COLOR_PAIR(pair));
      mvaddch(i, j, c);
      attroff(COLOR_PAIR(pair));
    }
  }
}

/* ------------------------------------------------------------------------- *
 * Panel layout: derived from the terminal size at startup.  The map stays a   *
 * fixed 80x21 in the top-left; any extra width becomes a party + legend panel *
 * on the right, and extra height becomes a message log across the bottom.     *
 * On a terminal too small for the panes they are simply skipped.              *
 * ------------------------------------------------------------------------- */
static int panel_x;     /* first column of the right panel */
static int panel_w;     /* right panel width */
static bool show_panel; /* enough columns for the right panel? */
static int log_y;       /* first row of the bottom message log */
static int log_h;       /* message log height */
static bool show_log;   /* enough rows for the log? */

static void compute_layout(void)
{
  panel_x = GRID_WIDTH + 1;
  panel_w = COLS - panel_x;
  show_panel = (panel_w >= 40);   /* a party line + legend need ~39 cols */

  log_y = GRID_HEIGHT + 1;
  log_h = LINES - log_y;
  show_log = (log_h >= 2);
}

/* Render a 10-segment HP bar into `out` (12 chars, NUL-terminated). */
static void hp_bar(char out[13], int cur, int max)
{
  int filled = (max > 0) ? (cur * 10) / max : 0;
  if (filled < 0)  filled = 0;
  if (filled > 10) filled = 10;
  out[0] = '[';
  for (int i = 0; i < 10; i++) out[1 + i] = (i < filled) ? '#' : '.';
  out[11] = ']';
  out[12] = '\0';
}

/* HP fraction -> display color: green >=50%, amber >=25%, else red. */
static int hp_color(int cur, int max)
{
  int pct;
  if (max <= 0) return CP_RED;
  pct = cur * 100 / max;
  if (pct >= 50) return CP_GREEN;
  if (pct >= 25) return CP_YELLOW;
  return CP_RED;
}

/* Legend glyph color mirrors the map: terrain via tile_pair(), NPCs cyan. */
static int legend_color(char c)
{
  switch (c) {
    case 'h': case 'r': case 'p': case 's':
    case 'w': case 'e': case 'm':
      return CP_CYAN;
    default:
      return tile_pair(c);
  }
}

static void colored_ch(int y, int x, char c)
{
  int p = legend_color(c);
  attron(COLOR_PAIR(p));
  mvaddch(y, x, c);
  attroff(COLOR_PAIR(p));
}

static void draw_box(int y, int x, int h, int w)
{
  mvhline(y, x, '-', w);
  mvhline(y + h - 1, x, '-', w);
  mvvline(y, x, '|', h);
  mvvline(y, x + w - 1, '|', h);
  mvaddch(y, x, '+');
  mvaddch(y, x + w - 1, '+');
  mvaddch(y + h - 1, x, '+');
  mvaddch(y + h - 1, x + w - 1, '+');
}

static void draw_party_panel(int y, int x, int w)
{
  draw_box(y, x, MAX_PARTY + 3, w);
  mvprintw(y + 1, x + 2, "PARTY %d/%d", party_count, MAX_PARTY);

  for (int i = 0; i < MAX_PARTY; i++) {
    if (i < party_count) {
      char bar[13];
      int pair = hp_color(party[i].cur_hp, party[i].hp);
      bool active = (i == party_active);
      hp_bar(bar, party[i].cur_hp, party[i].hp);
      if (active) attron(A_BOLD);
      mvprintw(y + 2 + i, x + 2, "%c%d) %-11s Lv.%-3d ",
               active ? '*' : ' ', i + 1,
               species_name(party[i].species), party[i].level);
      if (active) attroff(A_BOLD);
      attron(COLOR_PAIR(pair));
      printw("%s", bar);
      attroff(COLOR_PAIR(pair));
    } else {
      mvprintw(y + 2 + i, x + 2, " %d) %-11s", i + 1, "(empty)");
    }
  }
}

static void draw_legend_panel(int y, int x, int w)
{
  struct leg { char g; const char *label; };
  static const struct leg terrain[] = {
    { '%', "mountain" }, { '^', "tree" }, { ':', "grass" },
    { '~', "water" }, { '#', "path" }, { '.', "clear" },
    { 'M', "mart" }, { 'C', "center" }, { '@', "you" },
  };
  static const struct leg npcs[] = {
    { 'h', "hiker" }, { 'r', "rival" }, { 'p', "pacer" },
    { 's', "sentry" }, { 'w', "wanderer" }, { 'e', "explorer" },
    { 'm', "swimmer" },
  };
  int nter = (int)(sizeof terrain / sizeof terrain[0]);
  int nnpc = (int)(sizeof npcs / sizeof npcs[0]);

  draw_box(y, x, nter + 3, w);   /* header + 9 rows + 2 borders */
  mvprintw(y + 1, x + 2, "LEGEND");

  for (int i = 0; i < nter; i++) {
    colored_ch(y + 2 + i, x + 2, terrain[i].g);
    mvprintw(y + 2 + i, x + 4, "%-10s", terrain[i].label);
  }
  for (int i = 0; i < nnpc; i++) {
    colored_ch(y + 2 + i, x + 20, npcs[i].g);
    mvprintw(y + 2 + i, x + 22, "%-9s", npcs[i].label);
  }
}

static void draw_frame(terrain_map_t *g, Location_t loc)
{
  clear();
  print_map(g);

  /* Full-width reverse-video status bar. */
  char status[160];
  snprintf(status, sizeof status,
           " loc (%d,%d)  $%d  %d potions  %d balls  party %d/%d  [h help  m log  p party  q quit] ",
           loc.x - 200, loc.y - 200, bag.money, bag.potions, bag.pokeballs,
           party_count, MAX_PARTY);

  attron(A_REVERSE);
  mvprintw(GRID_HEIGHT, 0, "%s", status);
  for (int c = (int) strlen(status); c < COLS; c++) mvaddch(GRID_HEIGHT, c, ' ');
  attroff(A_REVERSE);

  if (show_panel) {
    draw_party_panel(0, panel_x, panel_w);
    draw_legend_panel(9, panel_x, panel_w);
  }

  if (show_log) {
    msg_show_log(log_y, log_h);
  }

  refresh();
}

/* Reset the current map to a fresh PC + NPC population and rebuild the
 * hiker/rival distance maps for the new PC position. */
static void setup_map(terrain_map_t *g, Location_t pc_entry)
{
  clear_characters(g);
  place_pc(g, pc_entry);
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

/* After leave_map(), `gates` holds exactly one non-zero door: the
 * connecting door on the map we are entering (the one the PC walked through). */
static Location_t entry_gate(Gates_t g)
{
  if (g.top.x || g.top.y)       return g.top;
  if (g.bottom.x || g.bottom.y) return g.bottom;
  if (g.left.x || g.left.y)     return g.left;
  return g.right;
}

/* ------------------------------------------------------------------------- *
 * Input.                                                                    *
 * ------------------------------------------------------------------------- */
static int read_int_prompt(const char *label)
{
  int v = 0;
  echo();
  curs_set(1);
  mvprintw(LINES - 1, 0, "%s", label);
  clrtoeol();
  refresh();
  mvscanw(LINES - 1, (int) strlen(label), "%d", &v);
  noecho();
  curs_set(0);
  return v;
}

static void print_help(void)
{
  clear();
  printw("========== ASCII-Pokemon : controls ==========\n\n");
  printw("  move          arrow keys, or n / s / e / w (one tile)\n");
  printw("  rest          . or any other key\n");
  printw("  fly           f  (then enter x and y, -200..200)\n");
  printw("  help          h or ?\n");
  printw("  message log   m\n");
  printw("  party         p\n");
  printw("  quit          q  (or Ctrl-C)\n\n");
  printw("  time passes by move cost: path=10 grass=20 water=7\n");
  printw("  The world is 401 x 401 maps; the center is (0,0).\n\n");
  printw("  @ you  %% mountain  ^ tree  : grass  ~ water  # path\n");
  printw("  M mart  C center\n");
  printw("  NPCs: h hiker  r rival  p pacer  w wanderer\n");
  printw("        s sentry  e explorer  m swimmer\n");
  printw("  walk in tall grass ':' to find wild pokemon\n");
  printw("  walk into a trainer to battle\n");
  printw("  step on 'C' to heal, 'M' to visit the mart\n\n");
  printw("press any key to return...\n");
  refresh();
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
  setup_map(currGrid, (Location_t) { 0, 0 });   /* first map: random spawn */

  /* Debug dump happens before ncurses takes over the terminal. */
  if (debug) {
    printf(" hiker cost map\n");
    print_dist_map(hiker_dist);
    printf("\nrival cost map\n");
    print_dist_map(rival_dist);
    printf("\npress enter to start...\n");
    getchar();
  }

  init_curses();
  compute_layout();

  /* The starter is the first (and active) party member. */
  starter_pick(&party[0], read_key_blocking);
  party_count = 1;
  party_active = 0;

  heap_t turn;
  heap_init(&turn, cmp_char_turn, NULL);
  rebuild_turn_queue(&turn);

  bool shouldQuit = false;
  draw_frame(currGrid, currLoc);

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
    draw_frame(currGrid, currLoc);

    int key = read_key_blocking();
    int dx = 0, dy = 0;
    bool move_requested = false;
    bool transitioned = false;

    switch (key) {
      case ERR:
      case 'q':
      case 'Q':
        shouldQuit = true;
        break;

      case 'h':
      case '?':
        print_help();
        break;   /* help costs no turn */

      case 'm':
        msg_hist_view(read_key_blocking);
        break;   /* viewing the log costs no turn */

      case 'p':
        party_view(read_key_blocking);
        break;   /* viewing the party costs no turn */

      case 'f': {
        int x = read_int_prompt("fly to x [-200, 200]: ");
        int y = read_int_prompt("fly to y [-200, 200]: ");
        if (x >= -200 && x <= 200 && y >= -200 && y <= 200) {
          currLoc.x = x + 200;
          currLoc.y = y + 200;
          gates = (Gates_t) { 0 };
          currGrid = load_map(worldMap, currLoc, gates);
          gates = currGrid->gates;
          setup_map(currGrid, (Location_t) { 0, 0 });   /* fly: random spawn */
          transitioned = true;
        } else {
          msg("You can't fly that far on your pidgey!");
        }
        break;
      }

      case 'n': case KEY_UP:    dx = -1; dy = 0;  move_requested = true; break;
      case 's': case KEY_DOWN:  dx =  1; dy = 0;  move_requested = true; break;
      case 'e': case KEY_RIGHT: dx =  0; dy = 1;  move_requested = true; break;
      case 'w': case KEY_LEFT:  dx =  0; dy = -1; move_requested = true; break;

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
        msg("That way is blocked.");
        pc.next_turn += REST_COST;
      } else if (res == PC_MOVE_BATTLE) {
        int tx = pc.location.x + dx, ty = pc.location.y + dy;
        struct character_s *foe = currGrid->cmap[tx][ty];
        if (foe) {
          int md = abs(currLoc.x - 200) + abs(currLoc.y - 200);
          trainer_battle(&party[party_active], read_key_blocking, foe, md);
        }
        pc.next_turn += REST_COST;
      } else if (res >= PC_MOVE_EXIT_UP) {
        currLoc = leave_map(res, currLoc, &gates);
        Location_t entry = entry_gate(gates);   /* the door we walked through */
        currGrid = load_map(worldMap, currLoc, gates);
        gates = currGrid->gates;
        setup_map(currGrid, entry);
        transitioned = true;
      } else {
        /* Moved within the map: buildings, then tall-grass encounters. */
        char t = currGrid->grid[pc.location.x][pc.location.y];
        if (t == pokemonCenter.ascii) {
          for (i = 0; i < party_count; i++) {
            party[i].cur_hp = party[i].hp;   /* the center heals the whole party */
          }
          msg("Nurse Joy healed your party!");
        } else if (t == pokeMart.ascii) {
          shop_mart(read_key_blocking);
        } else if (t == tallGrassTile.ascii &&
                   (rand() % 100) < ENCOUNTER_PROB) {
          int md = abs(currLoc.x - 200) + abs(currLoc.y - 200);
          wild_encounter(&party[party_active], read_key_blocking, md);
        }
      }
    }

    if (transitioned) {
      rebuild_turn_queue(&turn);
      continue;
    }

    /* Track the PC's live position so the hiker and rival actually chase
     * you, not the spot where you first spawned. */
    compute_dist_maps(currGrid, &pc.location);

    heap_insert(&turn, &pc);
  }

  endwin();
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