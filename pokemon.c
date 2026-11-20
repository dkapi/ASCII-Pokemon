#include <stdlib.h>

#include "pokemon.h"

const species_t pokemon_species[] = {
  { "Bulbasaur",  45,  49,  49,  65,  65,  45, 4 },
  { "Charmander", 39,  52,  43,  60,  50,  65, 2 },
  { "Squirtle",   44,  48,  65,  50,  64,  43, 3 },
  { "Pikachu",    35,  55,  40,  50,  50,  90, 5 },
  { "Caterpie",   45,  30,  35,  20,  20,  45, 0 },
  { "Pidgey",     40,  45,  40,  35,  35,  56, 6 },
  { "Rattata",    30,  56,  35,  25,  35,  72, 1 },
  { "Geodude",    40,  80, 100,  30,  30,  20, 7 },
  { "Zubat",      40,  45,  35,  30,  40,  55, 8 },
  { "Magikarp",   20,  10,  55,  15,  20,  80, 0 },
  { "Gyarados",   95, 125,  79,  60, 100,  81, 9 },
  { "Mewtwo",    106, 110,  90, 154,  90, 130, 9 },
};
const int pokemon_num_species =
  (int) (sizeof(pokemon_species) / sizeof(pokemon_species[0]));

const move_t pokemon_moves[] = {
  { "Tackle",        40 },
  { "Quick Attack",  40 },
  { "Ember",         40 },
  { "Water Gun",     40 },
  { "Vine Whip",     45 },
  { "Thunder Shock", 40 },
  { "Gust",          40 },
  { "Rock Throw",    50 },
  { "Bite",          60 },
  { "Hyper Beam",   150 },
};
const int pokemon_num_moves =
  (int) (sizeof(pokemon_moves) / sizeof(pokemon_moves[0]));

pokemon_t make_pokemon(int species, int level)
{
  const species_t *s = &pokemon_species[species];
  pokemon_t p;

  if (level < 1) level = 1;
  if (level > 100) level = 100;

  p.species = species;
  p.level = level;
  p.hp    = (s->base_hp    + (rand() & 0xf)) * 2 * level / 100 + level + 10;
  p.atk   = (s->base_atk   + (rand() & 0xf)) * 2 * level / 100 + 5;
  p.def   = (s->base_def   + (rand() & 0xf)) * 2 * level / 100 + 5;
  p.spatk = (s->base_spatk + (rand() & 0xf)) * 2 * level / 100 + 5;
  p.spdef = (s->base_spdef + (rand() & 0xf)) * 2 * level / 100 + 5;
  p.speed = (s->base_speed + (rand() & 0xf)) * 2 * level / 100 + 5;
  p.cur_hp = p.hp;
  p.move0 = 0;           /* Tackle */
  p.move1 = s->sig_move;

  return p;
}

const char *species_name(int idx)
{
  return pokemon_species[idx].name;
}

const char *move_name(int idx)
{
  return pokemon_moves[idx].name;
}

/* Wild pokemon level scales with distance from world center (200,200). */
int wild_level(int manhattan_dist)
{
  int lvl = manhattan_dist <= 200
            ? 1 + manhattan_dist / 20
            : 11 + (manhattan_dist - 200) / 10;
  if (lvl < 1) lvl = 1;
  if (lvl > 100) lvl = 100;
  return lvl;
}

/* Simplified damage: physical attack stat vs. physical defense. */
int battle_damage(const pokemon_t *atk, const pokemon_t *def, int move_idx)
{
  const move_t *m = &pokemon_moves[move_idx];
  int dmg;

  if (m->power == 0) return 0;

  dmg = ((2 * atk->level / 5 + 2) * m->power * atk->atk / def->def) / 50 + 2;
  dmg = dmg * (85 + rand() % 31) / 100;   /* ±15% variance */
  return dmg < 1 ? 1 : dmg;
}