#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>

#include "battle.h"
#include "msg.h"
#include "ui.h"
#include "sprites.h"

struct bag bag = { 3000, 5, 3, 0 };   /* $3000, 5 balls, 3 potions, 0 repels */
int repel_steps = 0;                  /* wild-encounter immunity remaining */

pokemon_t party[MAX_PARTY];
int party_count = 0;
int party_active = 0;

pokemon_t pc_box[MAX_BOX];
int pc_box_count = 0;

/* Low-HP wilds are easier to catch. */
static bool try_catch(const pokemon_t *wild)
{
  int pct = 25 + (100 - wild->cur_hp * 100 / wild->hp);
  if (pct > 100) pct = 100;
  if (pct < 25)  pct = 25;
  return (rand() % 100) < pct;
}

/* Map a type to a color pair for the type badge. */
static int type_color(int type)
{
  switch (type) {
    case TYPE_NORMAL: return CP_WHITE;
    case TYPE_FIGHTING:
    case TYPE_FIRE:   return CP_RED;
    case TYPE_FLYING:
    case TYPE_STEEL:
    case TYPE_ICE:    return CP_LBLUE;
    case TYPE_POISON:
    case TYPE_GHOST:
    case TYPE_PSYCHIC: return CP_MAGENTA;
    case TYPE_GROUND:
    case TYPE_ROCK:   return CP_BROWN;
    case TYPE_BUG:
    case TYPE_GRASS:  return CP_GREEN;
    case TYPE_WATER:
    case TYPE_DRAGON: return CP_BLUE;
    case TYPE_ELECTRIC: return CP_YELLOW;
    default:          return CP_WHITE;
  }
}

static void draw_sprite(int y, int x, int type)
{
  const char *const *s = type_sprites[type];
  for (int i = 0; i < SPRITE_H; i++) {
    mvprintw(y + i, x, "%s", s[i]);
  }
}

static void draw_type_badge(int y, int x, int type1, int type2)
{
  attron(COLOR_PAIR(type_color(type1)));
  mvprintw(y, x, "[%s", type_names[type1]);
  if (type2 != TYPE_NONE) {
    printw("/%s", type_names[type2]);
  }
  printw("]");
  attroff(COLOR_PAIR(type_color(type1)));
}

/* Fixed-layout battle screen: sprites + info panels + action bar + log. */
static void draw_battle_screen(pokemon_t *player, pokemon_t *enemy,
                               const char *enemy_label, bool can_catch,
                               int enemy_active, int enemy_team_size)
{
  char ebar[13], pbar[13], xbar[13];
  hp_bar(ebar, enemy->cur_hp, enemy->hp);
  hp_bar(pbar, player->cur_hp, player->hp);
  exp_bar(xbar, player->exp, player->exp_to_next);

  int et1 = pokemon_species[enemy->species].type1;
  int et2 = pokemon_species[enemy->species].type2;
  int pt1 = pokemon_species[player->species].type1;
  int pt2 = pokemon_species[player->species].type2;

  clear();

  /* Foe: sprite (top-left) + info (top-right). */
  draw_sprite(0, 2, et1);
  mvprintw(1, 14, "%s %s  Lv.%d", enemy_label, species_name(enemy->species),
           enemy->level);
  if (enemy_team_size > 1) {
    printw("  [%d/%d]", enemy_active + 1, enemy_team_size);
  }
  draw_type_badge(2, 14, et1, et2);
  attron(COLOR_PAIR(hp_color(enemy->cur_hp, enemy->hp)));
  mvprintw(3, 14, "HP %s %d/%d", ebar, enemy->cur_hp, enemy->hp);
  attroff(COLOR_PAIR(hp_color(enemy->cur_hp, enemy->hp)));

  /* You: sprite (bottom-left) + info (bottom-right). */
  draw_sprite(8, 2, pt1);
  mvprintw(9, 14, "Your %s  Lv.%d", species_name(player->species), player->level);
  draw_type_badge(10, 14, pt1, pt2);
  attron(COLOR_PAIR(hp_color(player->cur_hp, player->hp)));
  mvprintw(11, 14, "HP %s %d/%d", pbar, player->cur_hp, player->hp);
  attroff(COLOR_PAIR(hp_color(player->cur_hp, player->hp)));
  mvprintw(12, 14, "EXP %s %d/%d", xbar, player->exp, player->exp_to_next);

  /* Action bar. */
  mvhline(15, 0, '-', COLS);
  mvprintw(16, 2, "1) %-12s  2) %-12s  s)switch",
           move_name(player->move0), move_name(player->move1));
  mvprintw(17, 2, "p)potion(%d)", bag.potions);
  if (can_catch) {
    printw("  b)ball(%d)", bag.pokeballs);
  }
  printw("  r)run");

  /* Last two log lines. */
  msg_draw_line(19, 1, msg_recent(1), msg_recent_sev(1));
  msg_draw_line(20, 1, msg_recent(0), msg_recent_sev(0));

  mvprintw(17, 60, "> ");
  refresh();
}

/* Let the player pick a living party member to switch in.  Returns the index,
 * or -1 if cancelled. */
static int choose_party_member(int (*getkey)(void), int exclude)
{
  for (;;) {
    clear();
    printw("Choose a pokemon to switch in:\n");
    for (int i = 0; i < party_count; i++) {
      printw("  %d) %-16s Lv.%-3d HP %d/%d%s\n",
             i + 1, species_name(party[i].species), party[i].level,
             party[i].cur_hp, party[i].hp,
             (i == party_active) ? "  (active)" :
             (party[i].cur_hp <= 0) ? "  (fainted)" : "");
    }
    printw("  q) cancel\n> ");
    refresh();

    int k = getkey();
    if (k == 'q' || k == 'Q' || k < 0) return -1;
    if (k >= '1' && k <= '0' + party_count) {
      int idx = k - '1';
      if (idx != exclude && party[idx].cur_hp > 0) return idx;
    }
  }
}

static bool any_party_alive(void)
{
  for (int i = 0; i < party_count; i++) {
    if (party[i].cur_hp > 0) return true;
  }
  return false;
}

/* Award exp to the active pokemon for a win or catch, leveling up and
 * evolving as needed. */
static void award_exp(pokemon_t *player, int enemy_level)
{
  int old_species = player->species;
  int levels = gain_exp(player, enemy_level * enemy_level);

  if (player->species != old_species) {
    msg_good("What? %s is evolving into %s!",
             species_name(old_species), species_name(player->species));
  } else if (levels > 0) {
    msg_good("%s grew to Lv.%d!", species_name(player->species), player->level);
  }
}

/* One battle against a team of up to MAX_TEAM enemies.  Returns 2 = caught
 * (wild), 1 = won (all enemies defeated), 0 = lost or fled. */
static int run_battle(pokemon_t *enemy_team, int enemy_team_size,
                      int (*getkey)(void), const char *enemy_label, bool can_catch)
{
  int enemy_active = 0;

  for (;;) {
    pokemon_t *player = &party[party_active];
    pokemon_t *enemy = &enemy_team[enemy_active];

    draw_battle_screen(player, enemy, enemy_label, can_catch,
                       enemy_active, enemy_team_size);

    int k = getkey();
    while (k != 'r' && k != 'p' && k != 's' && k != '1' && k != '2' &&
           k >= 0 && !(can_catch && k == 'b')) {
      k = getkey();   /* ignore unrecognized keys */
    }

    if (k == 'r' || k < 0) {
      msg("You ran away!");
      return 0;
    }

    if (k == 's') {
      int idx = choose_party_member(getkey, party_active);
      if (idx >= 0) {
        party_active = idx;
        player = &party[party_active];
        msg("Go, %s!", species_name(player->species));
      } else {
        continue;   /* cancelled: no turn spent */
      }
    } else if (k == 'p') {
      if (bag.potions <= 0) { msg_bad("No potions left!"); continue; }
      if (player->cur_hp >= player->hp) { msg("Already at full HP!"); continue; }
      bag.potions--;
      player->cur_hp += POTION_HEAL;
      if (player->cur_hp > player->hp) player->cur_hp = player->hp;
      msg_good("Used a Potion! %s is back to %d/%d HP.",
               species_name(player->species), player->cur_hp, player->hp);
    } else if (can_catch && k == 'b') {
      if (bag.pokeballs <= 0) { msg_bad("No Poke Balls left!"); continue; }
      bag.pokeballs--;
      if (try_catch(enemy)) {
        msg_good("Gotcha! You caught %s!", species_name(enemy->species));
        award_exp(player, enemy->level);
        return 2;
      }
      msg_bad("The Poke Ball shook... %s broke free!", species_name(enemy->species));
    } else {
      /* A damaging move (k == '1' or '2'). */
      int move_idx = (k == '1') ? player->move0 : player->move1;
      double eff = move_effectiveness(move_idx, enemy->species);
      int dmg = battle_damage(player, enemy, move_idx);
      enemy->cur_hp -= dmg;
      msg("Your %s used %s, dealt %d damage!",
          species_name(player->species), move_name(move_idx), dmg);
      if (eff >= 2.0) {
        msg_good("It's super effective!");
      } else if (eff > 0.0 && eff < 1.0) {
        msg("It's not very effective...");
      } else if (eff <= 0.0) {
        msg_bad("It doesn't affect %s...", species_name(enemy->species));
      }
      if (enemy->cur_hp <= 0) {
        msg_good("Foe %s fainted!", species_name(enemy->species));
        award_exp(player, enemy->level);
        enemy_active++;
        if (enemy_active >= enemy_team_size) {
          return 1;   /* won */
        }
        msg("%s sent out %s!", enemy_label,
            species_name(enemy_team[enemy_active].species));
        continue;   /* next enemy, no enemy attack this round */
      }
    }

    /* Foe's turn. */
    int emove = (rand() & 1) ? enemy->move0 : enemy->move1;
    int edmg = battle_damage(enemy, player, emove);
    player->cur_hp -= edmg;
    msg_bad("Foe %s used %s, dealt %d damage!",
            species_name(enemy->species), move_name(emove), edmg);
    if (player->cur_hp <= 0) {
      msg_bad("Your %s fainted!", species_name(player->species));
      if (!any_party_alive()) {
        msg_bad("You have no more pokemon! You blacked out!");
        return -1;   /* lost: the caller relocates you to the Center */
      }
      int idx = choose_party_member(getkey, party_active);
      if (idx < 0) {
        for (int i = 0; i < party_count; i++) {
          if (party[i].cur_hp > 0) { idx = i; break; }
        }
      }
      party_active = idx;
      msg("Go, %s!", species_name(party[party_active].species));
    }
  }
}

void starter_pick(pokemon_t *out, int (*getkey)(void))
{
  for (;;) {
    clear();
    printw("Choose your starter (Gen 1 & 2):\n");
    for (int i = 0; i < NUM_STARTERS; i++) {
      printw("  %d) %s\n", i + 1, species_name(starter_species[i]));
    }
    printw("> ");
    refresh();

    int k = getkey();
    int idx = (k >= '1' && k <= '1' + NUM_STARTERS - 1) ? (k - '1') :
              (k < 0) ? 0 : -1;   /* EOF defaults to the first starter */
    if (idx >= 0) {
      *out = make_pokemon(starter_species[idx], 5);
      msg_good("You chose %s!", species_name(out->species));
      return;
    }
  }
}

/* Shared tail of a wild battle: a catch joins the party (or the box). */
static void finish_wild_battle(pokemon_t *wild, int res)
{
  if (res == 2) {
    wild->cur_hp = wild->hp;   /* caught at full HP */
    if (party_count < MAX_PARTY) {
      party[party_count++] = *wild;
      msg_good("%s joined your party! (%d/%d)",
               species_name(wild->species), party_count, MAX_PARTY);
    } else if (pc_box_count < MAX_BOX) {
      pc_box[pc_box_count++] = *wild;
      msg_good("%s was sent to the PC box!", species_name(wild->species));
    } else {
      msg_bad("Party and box are full - %s scurried away!",
              species_name(wild->species));
    }
  } else if (res == 1) {
    msg_good("The wild %s was defeated!", species_name(wild->species));
  }
}

/* Returns true if the player blacked out (lost). */
bool wild_encounter(int (*getkey)(void), int manhattan_dist, char tile)
{
  pokemon_t team[1];
  team[0] = make_pokemon(encounter_species(tile), wild_level(manhattan_dist));
  msg("A wild %s appeared!", species_name(team[0].species));

  int res = run_battle(team, 1, getkey, "Wild", true);
  finish_wild_battle(&team[0], res);
  return res == -1;
}

bool wild_encounter_cave(int (*getkey)(void), int manhattan_dist)
{
  int lvl = wild_level(manhattan_dist) + 5;   /* caves run hotter */
  if (lvl > 100) lvl = 100;

  pokemon_t team[1];
  team[0] = make_pokemon(cave_encounter_species(), lvl);
  msg("A wild %s appeared!", species_name(team[0].species));

  int res = run_battle(team, 1, getkey, "Wild", true);
  finish_wild_battle(&team[0], res);
  return res == -1;
}

/* Pick a species for the i-th member of a trainer's team, themed by class. */
static int trainer_species(int ctype, int slot)
{
  switch (ctype) {
    case CHAR_HIKER: {
      static const char *pool[] = { "Geodude", "Onix", "Machop", "Graveler" };
      return species_index(pool[slot % 4]);
    }
    case CHAR_SWIMMER: {
      static const char *pool[] = { "Magikarp", "Goldeen", "Staryu", "Tentacool" };
      return species_index(pool[slot % 4]);
    }
    case CHAR_RIVAL: {
      static const char *pool[] = { "Charmander", "Pidgey", "Rattata" };
      return species_index(pool[slot % 3]);
    }
    default:
      return rand() % pokemon_num_species;
  }
}

/* Returns true if the player blacked out (lost). */
bool trainer_battle(int (*getkey)(void), struct character_s *npc,
                    int manhattan_dist)
{
  const char *title = char_title(npc);

  /* Trainers scale with distance from the world center, a touch tougher
   * than the wild pokemon at the same distance. */
  int lvl = wild_level(manhattan_dist) + 2;
  if (lvl > 100) lvl = 100;

  /* Team size grows with distance: 2 near the center, 3 far out. */
  int team_size = 2 + (manhattan_dist >= 150 ? 1 : 0);
  pokemon_t team[MAX_TEAM];
  for (int i = 0; i < team_size; i++) {
    team[i] = make_pokemon(trainer_species(npc->ctype, i), lvl);
  }

  msg("A %s blocks your path!", title);

  int res = run_battle(team, team_size, getkey, title, false);
  if (res == 1) {
    npc->defeated = 1;
    npc->mtype = MOVE_SENTRY;   /* beaten trainers stand still */
    int reward = lvl * 50 * team_size;
    bag.money += reward;
    msg_good("You defeated the %s and won $%d!", title, reward);
  }
  return res == -1;
}

void party_view(int (*getkey)(void))
{
  for (;;) {
    clear();
    printw("========== Your party (%d/%d) ==========\n",
           party_count, MAX_PARTY);
    for (int i = 0; i < party_count; i++) {
      printw("  %s%d) %-16s  Lv.%-3d  HP %d/%d\n",
             (i == party_active) ? "*" : " ",
             i + 1, species_name(party[i].species),
             party[i].level, party[i].cur_hp, party[i].hp);
    }
    printw("\n  * = active   pick 1-%d to switch   q to return\n> ", party_count);
    refresh();

    int k = getkey();
    if (k == 'q' || k == 'Q' || k == ERR) {
      return;
    }
    if (k >= '1' && k <= '0' + party_count) {
      int idx = k - '1';
      if (idx != party_active) {
        party_active = idx;
        msg("Now battling with %s.", species_name(party[party_active].species));
      }
    }
  }
}

/* PC box: deposit the active party member, or withdraw from the box. */
void pc_box_view(int (*getkey)(void))
{
  for (;;) {
    clear();
    printw("========== PC Box (%d/%d) ==========\n", pc_box_count, MAX_BOX);
    printw("  Party:\n");
    for (int i = 0; i < party_count; i++) {
      printw("    %d) %-16s Lv.%-3d HP %d/%d%s\n",
             i + 1, species_name(party[i].species), party[i].level,
             party[i].cur_hp, party[i].hp,
             (i == party_active) ? "  (active)" : "");
    }
    printw("  Box:\n");
    for (int i = 0; i < pc_box_count; i++) {
      printw("    %d) %-16s Lv.%-3d HP %d/%d\n",
             i + 1, species_name(pc_box[i].species), pc_box[i].level,
             pc_box[i].cur_hp, pc_box[i].hp);
    }
    printw("\n  d) deposit active   w) withdraw   q) leave\n> ");
    refresh();

    int k = getkey();
    if (k == 'q' || k == 'Q' || k < 0) return;

    if (k == 'd') {
      if (party_count <= 1) { msg_bad("Can't deposit your last pokemon!"); continue; }
      if (pc_box_count >= MAX_BOX) { msg_bad("The box is full!"); continue; }
      pc_box[pc_box_count++] = party[party_active];
      for (int i = party_active; i < party_count - 1; i++) {
        party[i] = party[i + 1];
      }
      party_count--;
      if (party_active >= party_count) party_active = party_count - 1;
      msg_good("Deposited to the box.");
    } else if (k == 'w') {
      if (pc_box_count <= 0) { msg_bad("The box is empty!"); continue; }
      if (party_count >= MAX_PARTY) { msg_bad("Your party is full!"); continue; }

      int idx = -1;
      for (;;) {
        clear();
        printw("Withdraw which pokemon?\n");
        for (int i = 0; i < pc_box_count; i++) {
          printw("  %d) %-16s Lv.%-3d\n",
                 i + 1, species_name(pc_box[i].species), pc_box[i].level);
        }
        printw("  q) cancel\n> ");
        refresh();
        int k2 = getkey();
        if (k2 == 'q' || k2 == 'Q' || k2 < 0) break;
        if (k2 >= '1' && k2 <= '0' + pc_box_count) { idx = k2 - '1'; break; }
      }
      if (idx >= 0) {
        party[party_count++] = pc_box[idx];
        for (int i = idx; i < pc_box_count - 1; i++) pc_box[i] = pc_box[i + 1];
        pc_box_count--;
        msg_good("Withdrew %s!", species_name(party[party_count - 1].species));
      }
    }
  }
}

void shop_mart(int (*getkey)(void))
{
  for (;;) {
    clear();
    printw("========== Pokemon Mart ==========\n");
    printw("You have $%d\n", bag.money);
    printw("  1) Potion    ($%d)  heal %d HP   you have %d\n",
           POTION_COST, POTION_HEAL, bag.potions);
    printw("  2) Poke Ball ($%d)                you have %d\n",
           BALL_COST, bag.pokeballs);
    printw("  3) Repel     ($%d)  repel wilds %d steps  you have %d\n",
           REPEL_COST, REPEL_STEPS, bag.repels);
    printw("  q) leave\n> ");
    refresh();

    int k = getkey();
    while (k != 'q' && k != 'Q' && k != '1' && k != '2' && k != '3' && k >= 0) {
      k = getkey();   /* ignore stray keys so the menu isn't re-dumped */
    }
    if (k == 'q' || k == 'Q' || k < 0) {
      msg("Come again!");
      return;
    }
    if (k == '1') {
      if (bag.money >= POTION_COST) {
        bag.money -= POTION_COST;
        bag.potions++;
        msg_good("Bought a Potion!");
      } else {
        msg_bad("Not enough money!");
      }
    } else if (k == '2') {
      if (bag.money >= BALL_COST) {
        bag.money -= BALL_COST;
        bag.pokeballs++;
        msg_good("Bought a Poke Ball!");
      } else {
        msg_bad("Not enough money!");
      }
    } else if (k == '3') {
      if (bag.money >= REPEL_COST) {
        bag.money -= REPEL_COST;
        bag.repels++;
        msg_good("Bought a Repel!");
      } else {
        msg_bad("Not enough money!");
      }
    }
  }
}