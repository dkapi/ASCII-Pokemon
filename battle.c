#include <stdio.h>
#include <stdlib.h>

#include "battle.h"

static void pause_key(int (*getkey)(void))
{
  printf("(press any key to continue)\n");
  fflush(stdout);
  getkey();
}

/* One battle.  Returns 1 on victory, 0 on loss or flee. */
static int run_battle(pokemon_t *player, pokemon_t *enemy, int (*getkey)(void),
                      const char *enemy_label)
{
  for (;;) {
    printf("\n%s %s  (Lv.%d)  HP %d/%d\n",
           enemy_label, species_name(enemy->species), enemy->level,
           enemy->cur_hp, enemy->hp);
    printf("Your %s  (Lv.%d)  HP %d/%d\n",
           species_name(player->species), player->level,
           player->cur_hp, player->hp);
    printf("  1) %-14s 2) %-14s  r) run  > ",
           move_name(player->move0), move_name(player->move1));
    fflush(stdout);

    int k = getkey();
    int move_idx;

    if (k == 'r' || k < 0) {
      printf("\nYou ran away!\n");
      return 0;
    } else if (k == '1') {
      move_idx = player->move0;
    } else if (k == '2') {
      move_idx = player->move1;
    } else {
      continue;
    }

    int dmg = battle_damage(player, enemy, move_idx);
    enemy->cur_hp -= dmg;
    printf("  Your %s used %s, dealt %d damage!\n",
           species_name(player->species), move_name(move_idx), dmg);
    if (enemy->cur_hp <= 0) {
      printf("  Foe %s fainted! You win!\n", species_name(enemy->species));
      return 1;
    }

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
  static const int starters[3] = { 0, 1, 2 };   /* Bulbasaur/Charmander/Squirtle */

  for (;;) {
    printf("Choose your starter:\n");
    printf("  1) %-12s 2) %-12s 3) %s\n",
           species_name(starters[0]), species_name(starters[1]),
           species_name(starters[2]));
    printf("> ");
    fflush(stdout);

    int k = getkey();
    int idx = (k == '1') ? 0 : (k == '2') ? 1 : (k == '3') ? 2 :
              (k < 0) ? 0 : -1;   /* EOF defaults to the first starter */
    if (idx >= 0) {
      *out = make_pokemon(starters[idx], 5);
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

  if (run_battle(player, &wild, getkey, "Wild")) {
    printf("The wild %s was defeated!\n", species_name(wild.species));
  }
  pause_key(getkey);
}

void trainer_battle(pokemon_t *player, int (*getkey)(void),
                    struct character_s *npc)
{
  int sp;

  switch (npc->ctype) {
    case CHAR_HIKER:   sp = 7;  break;   /* Geodude */
    case CHAR_SWIMMER: sp = 9;  break;   /* Magikarp */
    case CHAR_RIVAL:   sp = 1;  break;   /* Charmander */
    default:           sp = rand() % pokemon_num_species; break;
  }

  pokemon_t enemy = make_pokemon(sp, 5 + rand() % 10);
  printf("\nA trainer with a %s blocks your path!\n", species_name(enemy.species));

  if (run_battle(player, &enemy, getkey, "Trainer") == 1) {
    npc->defeated = 1;
    npc->mtype = MOVE_WANDERER;   /* defeated trainers wander off */
  }
  pause_key(getkey);
}