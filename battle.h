#ifndef BATTLE_H
#define BATTLE_H

#include "pokemon.h"
#include "characters.h"

#define ENCOUNTER_PROB 10   /* % chance of a wild pokemon per tall-grass step */

#define POTION_COST 200
#define BALL_COST   100
#define REPEL_COST  350
#define POTION_HEAL  20
#define REPEL_STEPS 100   /* steps of wild-encounter immunity per Repel */

#define MAX_PARTY 6
#define MAX_TEAM  6    /* max enemy team size (trainers use 2-3) */
#define MAX_BOX   100  /* PC box capacity */

/* Player wallet + inventory. */
struct bag {
  int money;
  int pokeballs;
  int potions;
  int repels;
};
extern struct bag bag;

/* Steps of wild-encounter immunity remaining (set by using a Repel). */
extern int repel_steps;

/* The player's roguelike party: up to MAX_PARTY slots.  party[party_active]
 * is the one that fights; caught wilds are appended. */
extern pokemon_t party[MAX_PARTY];
extern int party_count;
extern int party_active;

/* PC box: overflow storage for caught pokemon when the party is full. */
extern pokemon_t pc_box[MAX_BOX];
extern int pc_box_count;

void starter_pick(pokemon_t *out, int (*getkey)(void));
/* Each returns true if the player blacked out (lost). */
bool wild_encounter(int (*getkey)(void), int manhattan_dist, char tile);
/* Cave encounter: cave-only pool, a few levels higher than the overworld. */
bool wild_encounter_cave(int (*getkey)(void), int manhattan_dist);
bool trainer_battle(int (*getkey)(void), struct character_s *npc,
                    int manhattan_dist);
void shop_mart(int (*getkey)(void));
void party_view(int (*getkey)(void));
void pc_box_view(int (*getkey)(void));   /* swap party <-> box at a Center */

#endif /* BATTLE_H */