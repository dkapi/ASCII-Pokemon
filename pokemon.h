#ifndef POKEMON_H
#define POKEMON_H

/* A small built-in Pokédex.  The course's full db_parse.c loaded 898 species
 * from the veekun CSVs under ~/.poke327; those data files aren't on this
 * machine, so we ship a representative subset inline. */

typedef struct species {
  char name[16];
  int base_hp, base_atk, base_def, base_spatk, base_spdef, base_speed;
  int sig_move;      /* a species-flavoured move (index into pokemon_moves) */
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

pokemon_t make_pokemon(int species, int level);
const char *species_name(int idx);
const char *move_name(int idx);
int wild_level(int manhattan_dist);
int battle_damage(const pokemon_t *atk, const pokemon_t *def, int move_idx);

#endif /* POKEMON_H */