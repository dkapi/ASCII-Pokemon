#include <stdlib.h>
#include <string.h>

#include "pokemon.h"

/* Move list — indexed by the MOVE_* enum in pokemon.h. */
const move_t pokemon_moves[] = {
  [MOVE_TACKLE]        = { "Tackle",        40 },
  [MOVE_QUICK_ATTACK]  = { "Quick Attack",  40 },
  [MOVE_EMBER]         = { "Ember",         40 },
  [MOVE_WATER_GUN]     = { "Water Gun",     40 },
  [MOVE_VINE_WHIP]     = { "Vine Whip",     45 },
  [MOVE_THUNDER_SHOCK] = { "Thunder Shock", 40 },
  [MOVE_GUST]          = { "Gust",          40 },
  [MOVE_ROCK_THROW]    = { "Rock Throw",    50 },
  [MOVE_BITE]          = { "Bite",          60 },
  [MOVE_HYPER_BEAM]    = { "Hyper Beam",   150 },
};
const int pokemon_num_moves = NUM_MOVES;

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
  p.move0 = MOVE_TACKLE;
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

int species_index(const char *name)
{
  for (int i = 0; i < pokemon_num_species; i++) {
    if (strcmp(pokemon_species[i].name, name) == 0) {
      return i;
    }
  }
  return 0;
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