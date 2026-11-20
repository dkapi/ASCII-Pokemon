#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "battle.h"

struct bag bag = { 3000, 5, 3 };   /* $3000, 5 balls, 3 potions to start */

static void pause_key(int (*getkey)(void))
{
  printf("(press any key to continue)\n");
  fflush(stdout);
  getkey();
}

/* Low-HP wilds are easier to catch. */
static bool try_catch(const pokemon_t *wild)
{
  int pct = 25 + (100 - wild->cur_hp * 100 / wild->hp);
  if (pct > 100) pct = 100;
  if (pct < 25)  pct = 25;
  return (rand() % 100) < pct;
}

/* One battle.  Returns 2 = caught, 1 = won, 0 = lost or fled. */
static int run_battle(pokemon_t *player, pokemon_t *enemy, int (*getkey)(void),
                      const char *enemy_label, bool can_catch)
{
  /* Print the status panel once per round; stray keys are swallowed below
   * so they don't re-dump the whole panel and scroll the map away. */
  for (;;) {
    printf("\n%s %s  (Lv.%d)  HP %d/%d\n",
           enemy_label, species_name(enemy->species), enemy->level,
           enemy->cur_hp, enemy->hp);
    printf("Your %s  (Lv.%d)  HP %d/%d\n",
           species_name(player->species), player->level,
           player->cur_hp, player->hp);
    printf("  1) %-14s 2) %-14s  p) Potion(%d)",
           move_name(player->move0), move_name(player->move1), bag.potions);
    if (can_catch) {
      printf("  b) Ball(%d)", bag.pokeballs);
    }
    printf("  r) run\n> ");
    fflush(stdout);

    int k = getkey();
    while (k != 'r' && k != 'p' && k != '1' && k != '2' &&
           k >= 0 && !(can_catch && k == 'b')) {
      k = getkey();   /* ignore unrecognized keys */
    }

    if (k == 'r' || k < 0) {
      printf("\nYou ran away!\n");
      return 0;
    }

    if (k == 'p') {
      if (bag.potions <= 0) {
        printf("  (no potions left)\n");
        continue;
      }
      if (player->cur_hp >= player->hp) {
        printf("  (already at full HP)\n");
        continue;
      }
      bag.potions--;
      player->cur_hp += POTION_HEAL;
      if (player->cur_hp > player->hp) player->cur_hp = player->hp;
      printf("  Used a Potion! %s is at %d/%d HP.\n",
             species_name(player->species), player->cur_hp, player->hp);
    } else if (can_catch && k == 'b') {
      if (bag.pokeballs <= 0) {
        printf("  (no Poke Balls left)\n");
        continue;
      }
      bag.pokeballs--;
      if (try_catch(enemy)) {
        printf("\nGotcha! You caught %s!\n", species_name(enemy->species));
        return 2;
      }
      printf("  The Poke Ball shook... %s broke free!\n",
             species_name(enemy->species));
    } else {
      /* A damaging move (k == '1' or '2'). */
      int move_idx = (k == '1') ? player->move0 : player->move1;
      int dmg = battle_damage(player, enemy, move_idx);
      enemy->cur_hp -= dmg;
      printf("  Your %s used %s, dealt %d damage!\n",
             species_name(player->species), move_name(move_idx), dmg);
      if (enemy->cur_hp <= 0) {
        printf("  Foe %s fainted! You win!\n", species_name(enemy->species));
        return 1;
      }
    }

    /* Foe's turn. */
    int emove = (rand() & 1) ? enemy->move0 : enemy->move1;
    int edmg = battle_damage(enemy, player, emove);
    player->cur_hp -= edmg;
    printf("  Foe %s used %s, dealt %d damage!\n",
           species_name(enemy->species), move_name(emove), edmg);
    if (player->cur_hp <= 0) {
      printf("  Your %s fainted! You were rushed to the Pokemon Center..\n",
             species_name(player->species));
      player->cur_hp = player->hp;   /* heal up */
      return 0;
    }
  }
}

void starter_pick(pokemon_t *out, int (*getkey)(void))
{
  for (;;) {
    printf("Choose your starter (Gen 1 & 2):\n");
    for (int i = 0; i < NUM_STARTERS; i++) {
      printf("  %d) %s\n", i + 1, species_name(starter_species[i]));
    }
    printf("> ");
    fflush(stdout);

    int k = getkey();
    int idx = (k >= '1' && k <= '1' + NUM_STARTERS - 1) ? (k - '1') :
              (k < 0) ? 0 : -1;   /* EOF defaults to the first starter */
    if (idx >= 0) {
      *out = make_pokemon(starter_species[idx], 5);
      printf("\nYou chose %s!\n", species_name(out->species));
      return;
    }
  }
}

void wild_encounter(pokemon_t *player, int (*getkey)(void), int manhattan_dist)
{
  pokemon_t wild = make_pokemon(rand() % pokemon_num_species,
                                wild_level(manhattan_dist));
  printf("\nA wild %s appeared!\n", species_name(wild.species));

  int res = run_battle(player, &wild, getkey, "Wild", true);
  if (res == 2) {
    printf("%s was caught! (no party system yet, so it's a trophy)\n",
           species_name(wild.species));
  } else if (res == 1) {
    printf("The wild %s was defeated!\n", species_name(wild.species));
  }
  pause_key(getkey);
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

  /* Trainers scale with distance from the world center, a touch tougher
   * than the wild pokemon at the same distance. */
  int lvl = wild_level(manhattan_dist) + 2;
  if (lvl > 100) lvl = 100;

  pokemon_t enemy = make_pokemon(sp, lvl);
  printf("\nA trainer with a %s blocks your path!\n", species_name(enemy.species));

  if (run_battle(player, &enemy, getkey, "Trainer", false) == 1) {
    npc->defeated = 1;
    npc->mtype = MOVE_WANDERER;   /* defeated trainers wander off */
    int reward = enemy.level * 50;
    bag.money += reward;
    printf("\nYou won $%d!\n", reward);
  }
  pause_key(getkey);
}

void shop_mart(int (*getkey)(void))
{
  for (;;) {
    printf("\n========== Pokemon Mart ==========\n");
    printf("You have $%d\n", bag.money);
    printf("  1) Potion    ($%d)  heal %d HP   you have %d\n",
           POTION_COST, POTION_HEAL, bag.potions);
    printf("  2) Poke Ball ($%d)                you have %d\n",
           BALL_COST, bag.pokeballs);
    printf("  q) leave\n> ");
    fflush(stdout);

    int k = getkey();
    while (k != 'q' && k != 'Q' && k != '1' && k != '2' && k >= 0) {
      k = getkey();   /* ignore stray keys so the menu isn't re-dumped */
    }
    if (k == 'q' || k == 'Q' || k < 0) {
      printf("\nCome again!\n");
      return;
    }
    if (k == '1') {
      if (bag.money >= POTION_COST) {
        bag.money -= POTION_COST;
        bag.potions++;
        printf("\nBought a Potion!\n");
      } else {
        printf("\nNot enough money!\n");
      }
    } else if (k == '2') {
      if (bag.money >= BALL_COST) {
        bag.money -= BALL_COST;
        bag.pokeballs++;
        printf("\nBought a Poke Ball!\n");
      } else {
        printf("\nNot enough money!\n");
      }
    }
  }
}