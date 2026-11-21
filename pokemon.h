#ifndef POKEMON_H
#define POKEMON_H

/* Generation 1-2 Pokédex (251 species).  The species table lives in
 * pokedex_gen12.c, auto-generated from the veekun pokedex CSVs. */

/* Move indices — keep in sync with pokemon_moves[] in pokemon.c. */
enum {
  MOVE_TACKLE = 0,
  MOVE_QUICK_ATTACK,
  MOVE_EMBER,
  MOVE_WATER_GUN,
  MOVE_VINE_WHIP,
  MOVE_THUNDER_SHOCK,
  MOVE_GUST,
  MOVE_ROCK_THROW,
  MOVE_BITE,
  MOVE_HYPER_BEAM,
  NUM_MOVES,
};

/* Gen 1: Bulbasaur/Charmander/Squirtle, Gen 2: Chikorita/Cyndaquil/Totodile */
#define NUM_STARTERS 6

typedef struct species {
  char name[16];
  int base_hp, base_atk, base_def, base_spatk, base_spdef, base_speed;
  int sig_move;      /* species-flavoured move (a MOVE_* index) */
} species_t;

typedef struct move {
  char name[16];
  int power;         /* 0 = non-damaging */
} move_t;

typedef struct pokemon {
  int species;       /* index into pokemon_species[] */
  int level;
  int hp, atk, def, spatk, spdef, speed;
  int cur_hp;
  int move0;         /* always Tackle */
  int move1;         /* the species' signature move */
} pokemon_t;

extern const species_t pokemon_species[];
extern const int pokemon_num_species;
extern const move_t pokemon_moves[];
extern const int pokemon_num_moves;
extern const int starter_species[NUM_STARTERS];

pokemon_t make_pokemon(int species, int level);
const char *species_name(int idx);
const char *move_name(int idx);
int species_index(const char *name);
int wild_level(int manhattan_dist);
int battle_damage(const pokemon_t *atk, const pokemon_t *def, int move_idx);

/* Encounter table: % chance a step on `tile` triggers a wild encounter. */
int encounter_prob(char tile);
/* A random species appropriate to `tile` (fallback: any species). */
int encounter_species(char tile);

#endif /* POKEMON_H */