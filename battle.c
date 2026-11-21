#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>

#include "battle.h"
#include "msg.h"
#include "ui.h"

struct bag bag = { 3000, 5, 3 };   /* $3000, 5 balls, 3 potions to start */

pokemon_t party[MAX_PARTY];
int party_count = 0;
int party_active = 0;

/* Low-HP wilds are easier to catch. */
static bool try_catch(const pokemon_t *wild)
{
  int pct = 25 + (100 - wild->cur_hp * 100 / wild->hp);
  if (pct > 100) pct = 100;
  if (pct < 25)  pct = 25;
  return (rand() % 100) < pct;
}

/* Fixed-layout battle screen: redrawn in place each round (no scrolling). */
static void draw_battle_screen(pokemon_t *player, pokemon_t *enemy,
                               const char *enemy_label, bool can_catch)
{
  char ebar[13], pbar[13];
  hp_bar(ebar, enemy->cur_hp, enemy->hp);
  hp_bar(pbar, player->cur_hp, player->hp);

  clear();
  mvprintw(0, 0, "%s %s  (Lv.%d)   HP %d/%d ",
           enemy_label, species_name(enemy->species), enemy->level,
           enemy->cur_hp, enemy->hp);
  attron(COLOR_PAIR(hp_color(enemy->cur_hp, enemy->hp)));
  printw("%s", ebar);
  attroff(COLOR_PAIR(hp_color(enemy->cur_hp, enemy->hp)));

  mvprintw(1, 0, "Your %s  (Lv.%d)   HP %d/%d ",
           species_name(player->species), player->level,
           player->cur_hp, player->hp);
  attron(COLOR_PAIR(hp_color(player->cur_hp, player->hp)));
  printw("%s", pbar);
  attroff(COLOR_PAIR(hp_color(player->cur_hp, player->hp)));

  mvprintw(3, 0, "  1) %-14s  2) %-14s  p) Potion(%d)",
           move_name(player->move0), move_name(player->move1), bag.potions);
  if (can_catch) {
    printw("  b) Ball(%d)", bag.pokeballs);
  }
  printw("  r) run  > ");

  /* The last two events, newest on the bottom line, colored by severity. */
  msg_draw_line(5, 0, msg_recent(1), msg_recent_sev(1));
  msg_draw_line(6, 0, msg_recent(0), msg_recent_sev(0));
  refresh();
}

/* One battle.  Returns 2 = caught, 1 = won, 0 = lost or fled. */
static int run_battle(pokemon_t *player, pokemon_t *enemy, int (*getkey)(void),
                      const char *enemy_label, bool can_catch)
{
  for (;;) {
    draw_battle_screen(player, enemy, enemy_label, can_catch);

    int k = getkey();
    while (k != 'r' && k != 'p' && k != '1' && k != '2' &&
           k >= 0 && !(can_catch && k == 'b')) {
      k = getkey();   /* ignore unrecognized keys */
    }

    if (k == 'r' || k < 0) {
      msg("You ran away!");
      return 0;
    }

    if (k == 'p') {
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
        return 2;
      }
      msg_bad("The Poke Ball shook... %s broke free!", species_name(enemy->species));
    } else {
      /* A damaging move (k == '1' or '2'). */
      int move_idx = (k == '1') ? player->move0 : player->move1;
      int dmg = battle_damage(player, enemy, move_idx);
      enemy->cur_hp -= dmg;
      msg("Your %s used %s, dealt %d damage!",
          species_name(player->species), move_name(move_idx), dmg);
      if (enemy->cur_hp <= 0) {
        msg_good("Foe %s fainted! You win!", species_name(enemy->species));
        return 1;
      }
    }

    /* Foe's turn. */
    int emove = (rand() & 1) ? enemy->move0 : enemy->move1;
    int edmg = battle_damage(enemy, player, emove);
    player->cur_hp -= edmg;
    msg_bad("Foe %s used %s, dealt %d damage!",
            species_name(enemy->species), move_name(emove), edmg);
    if (player->cur_hp <= 0) {
      msg_bad("Your %s fainted! You were rushed to the Pokemon Center.",
              species_name(player->species));
      player->cur_hp = player->hp;   /* heal up */
      return 0;
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

void wild_encounter(pokemon_t *player, int (*getkey)(void), int manhattan_dist,
                    char tile)
{
  pokemon_t wild = make_pokemon(encounter_species(tile),
                                wild_level(manhattan_dist));
  msg("A wild %s appeared!", species_name(wild.species));

  int res = run_battle(player, &wild, getkey, "Wild", true);
  if (res == 2) {
    if (party_count < MAX_PARTY) {
      party[party_count++] = wild;
      msg_good("%s joined your party! (%d/%d)",
               species_name(wild.species), party_count, MAX_PARTY);
    } else {
      msg_bad("Party is full - %s scurried away!", species_name(wild.species));
    }
  } else if (res == 1) {
    msg_good("The wild %s was defeated!", species_name(wild.species));
  }
}

void trainer_battle(pokemon_t *player, int (*getkey)(void),
                    struct character_s *npc, int manhattan_dist)
{
  int sp;

  switch (npc->ctype) {
    case CHAR_HIKER:   sp = species_index("Geodude");   break;
    case CHAR_SWIMMER: sp = species_index("Magikarp");  break;
    case CHAR_RIVAL:   sp = species_index("Charmander"); break;
    default:           sp = rand() % pokemon_num_species; break;
  }

  /* Identify the trainer by their class, not their pokemon. */
  const char *title = char_title(npc);

  /* Trainers scale with distance from the world center, a touch tougher
   * than the wild pokemon at the same distance. */
  int lvl = wild_level(manhattan_dist) + 2;
  if (lvl > 100) lvl = 100;

  pokemon_t enemy = make_pokemon(sp, lvl);
  msg("A %s blocks your path!", title);

  if (run_battle(player, &enemy, getkey, title, false) == 1) {
    npc->defeated = 1;
    npc->mtype = MOVE_SENTRY;   /* beaten trainers stand still */
    int reward = enemy.level * 50;
    bag.money += reward;
    msg_good("You defeated the %s and won $%d!", title, reward);
  }
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
    printw("  q) leave\n> ");
    refresh();

    int k = getkey();
    while (k != 'q' && k != 'Q' && k != '1' && k != '2' && k >= 0) {
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
    }
  }
}