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
#include "ui.h"
#include "cave.h"

/* Cave-layer state: whether the player is underground, plus the overworld
 * spot they descended from, so ascending always returns home. */
static bool in_cave = false;
static int  vision_radius = 0;             /* 0 = full light (overworld) */
static Location_t cave_home;               /* overworld map coord descended from */
static Location_t cave_home_tile;          /* the 'O' tile descended through */

/* ------------------------------------------------------------------------- *
 * ncurses initialization (color pairs are defined in ui.h).                 *
 * ------------------------------------------------------------------------- */
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
    case '=': return CP_YELLOW;   /* sand */
    case '*': return CP_WHITE;    /* snow */
    case 'C': return CP_RED;
    case 'M': return CP_LBLUE;
    case 'O': return CP_MAGENTA;  /* cave entrance */
    case '>': return CP_WHITE;    /* cave exit stair */
    case '$': return CP_YELLOW;   /* loot: money */
    case '!': return CP_CYAN;     /* loot: item */
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

      /* Cave darkness: hide every cell outside the circular torch radius. */
      if (vision_radius > 0) {
        int dx = i - pc.location.x;
        int dy = j - pc.location.y;
        if (dx * dx + dy * dy > vision_radius * vision_radius) {
          mvaddch(i, j, ' ');
          continue;
        }
      }

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
  static const struct leg left[] = {
    { '%', "mountain" }, { '^', "tree" }, { ':', "grass" },
    { '~', "water" }, { '#', "path" }, { '.', "clear" },
    { '=', "sand" }, { '*', "snow" }, { 'O', "cave in" },
    { '>', "cave out" }, { '$', "loot" }, { '@', "you" },
  };
  static const struct leg right[] = {
    { 'M', "mart" }, { 'C', "center" },
    { 'h', "hiker" }, { 'r', "rival" }, { 'p', "pacer" },
    { 's', "sentry" }, { 'w', "wanderer" }, { 'e', "explorer" },
    { 'm', "swimmer" },
  };
  int nl = (int)(sizeof left / sizeof left[0]);
  int nr = (int)(sizeof right / sizeof right[0]);

  draw_box(y, x, nl + 3, w);   /* header + 9 rows + 2 borders */
  mvprintw(y + 1, x + 2, "LEGEND");

  for (int i = 0; i < nl; i++) {
    colored_ch(y + 2 + i, x + 2, left[i].g);
    mvprintw(y + 2 + i, x + 4, "%-10s", left[i].label);
  }
  for (int i = 0; i < nr; i++) {
    colored_ch(y + 2 + i, x + 20, right[i].g);
    mvprintw(y + 2 + i, x + 22, "%-9s", right[i].label);
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

/* Caves have no trainers: place only the PC.  With a door (sideways tunnel)
 * spawn on it; otherwise drop onto a random floor cell. */
static void setup_map_cave(terrain_map_t *g, Location_t entry)
{
  clear_characters(g);
  if (entry.x || entry.y) {
    place_pc(g, entry);
  } else {
    Location_t l;
    int attempts = 0;
    do {
      l.x = 1 + rand() % (GRID_HEIGHT - 3);
      l.y = 1 + rand() % (GRID_WIDTH - 3);
    } while (g->grid[l.x][l.y] != CAVE_FLOOR && ++attempts < 10000);
    g->cmap[l.x][l.y] = &pc;
    pc.location.x = l.x;
    pc.location.y = l.y;
    pc.next_turn = 0;
    pc.seq_num = NUM_NPCS;
  }
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

/* Load an already-generated cave map, or generate it, at world location loc.
 * The cave grid lives in a parallel cache so overworld and cave maps never
 * overwrite each other. */
static terrain_map_t *load_cave_map(terrain_map_t *(*worldMapCave)[WORLD_WIDTH],
                                    Location_t loc, Gates_t gates)
{
  if (!worldMapCave[loc.x][loc.y]) {
    terrain_map_t *g = malloc(sizeof *g);
    assert(g);
    g->location = loc;
    generate_cave_map(g, gates);
    worldMapCave[loc.x][loc.y] = g;
  }
  return worldMapCave[loc.x][loc.y];
}

static void rebuild_turn_queue(heap_t *turn)
{
  heap_delete(turn);
  heap_init(turn, cmp_char_turn, NULL);

  pc.next_turn = 0;
  heap_insert(turn, &pc);

  if (in_cave) return;   /* caves have no trainers; the PC acts alone */

  for (int i = 0; i < NUM_NPCS; i++) {
    npc_list[i]->next_turn = 0;
    heap_insert(turn, npc_list[i]);
  }
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
  printw("  step on 'C' to heal, 'M' to visit the mart\n");
  printw("  'O' is a cave entrance: descend into a dark cavern\n");
  printw("    (limited torch light, stronger cave-only wilds)\n");
  printw("    pick up '$' money and '!' items, find '>' to climb out\n\n");
  printw("press any key to return...\n");
  refresh();
  wait_any_key();
}

/* ------------------------------------------------------------------------- *
 * Title screen.                                                             *
 * ------------------------------------------------------------------------- */
static void center_text(int y, const char *s)
{
  int x = (COLS - (int) strlen(s)) / 2;
  if (x < 0) x = 0;
  mvprintw(y, x, "%s", s);
}

/* Returns true if the player chose to quit from the title. */
static bool show_start_screen(void)
{
  static const char *ascii_title[] = {
    " ###   ####  #### ##### ##### ",
    "#   # #     #       #     #   ",
    "#####  ###  #       #     #   ",
    "#   #     # #       #     #   ",
    "#   # ####   #### ##### ##### ",
  };
  static const char *pokemon_title[] = {
    "#####  ###  #   # ##### #   #  ###  #   # ",
    "#   # #   # #  #  #     ## ## #   # ##  # ",
    "#   # #   # ##    ####  # # # #   # # # # ",
    "##### #   # #  #  #     #   # #   # #  ## ",
    "#      ###  #   # ##### #   #  ###  #   # ",
  };

  int cy = (LINES - 17) / 2;
  if (cy < 0) cy = 0;
  int ax = (COLS - 30) / 2;
  if (ax < 0) ax = 0;
  int px = (COLS - 42) / 2;
  if (px < 0) px = 0;

  clear();
  for (int i = 0; i < 5; i++) mvprintw(cy + i, ax, "%s", ascii_title[i]);

  attron(COLOR_PAIR(CP_YELLOW));
  for (int i = 0; i < 5; i++) mvprintw(cy + 6 + i, px, "%s", pokemon_title[i]);
  attroff(COLOR_PAIR(CP_YELLOW));

  center_text(cy + 12, "a tiny pocket-monster roguelike");
  center_text(cy + 13, "walk the tall grass : catch 'em, battle 'em");

  attron(A_BOLD);
  center_text(cy + 15, "press any key to begin");
  attroff(A_BOLD);
  center_text(cy + 16, "(q to quit)");

  refresh();
  int k = getch();
  return (k == 'q' || k == 'Q' || k == ERR);
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
    } else if (!strcmp(argv[i], "--voronoi")) {
      terrain_noise = false;
    } else if (!strcmp(argv[i], "--no-warp")) {
      terrain_warp = false;
    }
  }

  srand(time(NULL));

  terrain_map_t *(*worldMap)[WORLD_WIDTH] =
    calloc(WORLD_HEIGHT, sizeof *worldMap);
  assert(worldMap);

  terrain_map_t *(*worldMapCave)[WORLD_WIDTH] =
    calloc(WORLD_HEIGHT, sizeof *worldMapCave);
  assert(worldMapCave);

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

  if (show_start_screen()) {
    endwin();
    for (i = 0; i < WORLD_HEIGHT; i++) {
      for (j = 0; j < WORLD_WIDTH; j++) {
        free(worldMap[i][j]);
        free(worldMapCave[i][j]);
      }
    }
    free(worldMap);
    free(worldMapCave);
    return 0;
  }

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
          msg_bad("You can't fly that far on your pidgey!");
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
        if (in_cave) {
          currGrid = load_cave_map(worldMapCave, currLoc, gates);
          gates = currGrid->gates;
          setup_map_cave(currGrid, entry);
        } else {
          currGrid = load_map(worldMap, currLoc, gates);
          gates = currGrid->gates;
          setup_map(currGrid, entry);
        }
        transitioned = true;
      } else {
        /* Moved within the map. */
        char t = currGrid->grid[pc.location.x][pc.location.y];
        int tx = pc.location.x, ty = pc.location.y;

        if (in_cave) {
          if (t == CAVE_EXIT) {
            /* climb out, back to the overworld we descended from */
            in_cave = false;
            vision_radius = 0;
            currLoc = cave_home;
            currGrid = load_map(worldMap, currLoc, (Gates_t) { 0 });
            gates = currGrid->gates;
            setup_map(currGrid, cave_home_tile);
            transitioned = true;
          } else if (t == CAVE_MONEY) {
            int amt = 200 + rand() % 401;
            bag.money += amt;
            currGrid->grid[tx][ty] = CAVE_FLOOR;
            msg_good("You found $%d in the gloom!", amt);
          } else if (t == CAVE_ITEM) {
            if (rand() & 1) { bag.potions += 2; msg_good("You found 2 potions!"); }
            else { bag.pokeballs += 2; msg_good("You found 2 pokeballs!"); }
            currGrid->grid[tx][ty] = CAVE_FLOOR;
          } else if ((rand() % 100) < CAVE_ENCOUNTER_PROB) {
            int md = abs(currLoc.x - 200) + abs(currLoc.y - 200);
            wild_encounter_cave(&party[party_active], read_key_blocking, md);
          }
        } else {
          if (t == pokemonCenter.ascii) {
            for (i = 0; i < party_count; i++) {
              party[i].cur_hp = party[i].hp;   /* the center heals the whole party */
            }
            msg_good("Nurse Joy healed your party!");
          } else if (t == pokeMart.ascii) {
            shop_mart(read_key_blocking);
          } else if (t == caveEntranceTile.ascii) {
            /* descend into the cave under this map */
            cave_home = currLoc;
            cave_home_tile = pc.location;
            in_cave = true;
            vision_radius = CAVE_VISION;
            currGrid = load_cave_map(worldMapCave, currLoc, (Gates_t) { 0 });
            gates = currGrid->gates;
            setup_map_cave(currGrid, (Location_t) { 0, 0 });
            transitioned = true;
          } else {
            int prob = encounter_prob(t);
            if (prob > 0 && (rand() % 100) < prob) {
              int md = abs(currLoc.x - 200) + abs(currLoc.y - 200);
              wild_encounter(&party[party_active], read_key_blocking, md, t);
            }
          }
        }
      }
    }

    if (transitioned) {
      rebuild_turn_queue(&turn);
      continue;
    }

    /* Track the PC's live position so the hiker and rival actually chase
     * you, not the spot where you first spawned.  (Skipped in caves, which
     * have no trainers to chase.) */
    if (!in_cave) {
      compute_dist_maps(currGrid, &pc.location);
    }

    heap_insert(&turn, &pc);
  }

  endwin();
  printf("But how are you going to be the very best if you quit..\n");

  heap_delete(&turn);   /* free the remaining turn-queue nodes */

  for (i = 0; i < WORLD_HEIGHT; i++) {
    for (j = 0; j < WORLD_WIDTH; j++) {
      free(worldMap[i][j]);
      free(worldMapCave[i][j]);
    }
  }
  free(worldMap);
  free(worldMapCave);

  return 0;
}