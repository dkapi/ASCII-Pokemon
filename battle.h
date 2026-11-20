#ifndef BATTLE_H
#define BATTLE_H

#include "pokemon.h"
#include "characters.h"

#define ENCOUNTER_PROB 10   /* % chance of a wild pokemon per tall-grass step */

void starter_pick(pokemon_t *out, int (*getkey)(void));
void wild_encounter(pokemon_t *player, int (*getkey)(void), int manhattan_dist);
void trainer_battle(pokemon_t *player, int (*getkey)(void),
                    struct character_s *npc);

#endif /* BATTLE_H */