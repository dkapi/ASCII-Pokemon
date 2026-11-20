#ifndef BATTLE_H
#define BATTLE_H

#include "pokemon.h"
#include "characters.h"

#define ENCOUNTER_PROB 10   /* % chance of a wild pokemon per tall-grass step */

#define POTION_COST 200
#define BALL_COST   100
#define POTION_HEAL  20

/* Player wallet + inventory. */
struct bag {
  int money;
  int pokeballs;
  int potions;
};
extern struct bag bag;

void starter_pick(pokemon_t *out, int (*getkey)(void));
void wild_encounter(pokemon_t *player, int (*getkey)(void), int manhattan_dist);
void trainer_battle(pokemon_t *player, int (*getkey)(void),
                    struct character_s *npc, int manhattan_dist);
void shop_mart(int (*getkey)(void));

#endif /* BATTLE_H */