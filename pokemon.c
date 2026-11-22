#include <stdlib.h>
#include <string.h>

#include "pokemon.h"

/* Move list — indexed by the MOVE_* enum in pokemon.h. */
const move_t pokemon_moves[] = {
  [MOVE_TACKLE]        = { "Tackle",        40, TYPE_NORMAL },
  [MOVE_QUICK_ATTACK]  = { "Quick Attack",  40, TYPE_NORMAL },
  [MOVE_EMBER]         = { "Ember",         40, TYPE_FIRE },
  [MOVE_WATER_GUN]     = { "Water Gun",     40, TYPE_WATER },
  [MOVE_VINE_WHIP]     = { "Vine Whip",     45, TYPE_GRASS },
  [MOVE_THUNDER_SHOCK] = { "Thunder Shock", 40, TYPE_ELECTRIC },
  [MOVE_GUST]          = { "Gust",          40, TYPE_FLYING },
  [MOVE_ROCK_THROW]    = { "Rock Throw",    50, TYPE_ROCK },
  [MOVE_BITE]          = { "Bite",          60, TYPE_DARK },
  [MOVE_HYPER_BEAM]    = { "Hyper Beam",   150, TYPE_NORMAL },
};
const int pokemon_num_moves = NUM_MOVES;

/* Recompute a pokemon's stats from its base stats at its current level.
 * Uses a fixed IV of 8 so level-ups are deterministic (no stat re-roll). */
static void compute_stats(pokemon_t *p)
{
  const species_t *s = &pokemon_species[p->species];
  int lvl = p->level;
  p->hp    = (s->base_hp    + 8) * 2 * lvl / 100 + lvl + 10;
  p->atk   = (s->base_atk   + 8) * 2 * lvl / 100 + 5;
  p->def   = (s->base_def   + 8) * 2 * lvl / 100 + 5;
  p->spatk = (s->base_spatk + 8) * 2 * lvl / 100 + 5;
  p->spdef = (s->base_spdef + 8) * 2 * lvl / 100 + 5;
  p->speed = (s->base_speed + 8) * 2 * lvl / 100 + 5;
}

pokemon_t make_pokemon(int species, int level)
{
  const species_t *s = &pokemon_species[species];
  pokemon_t p;

  if (level < 1) level = 1;
  if (level > 100) level = 100;

  p.species = species;
  p.level = level;
  compute_stats(&p);
  p.cur_hp = p.hp;
  p.exp = 0;
  p.exp_to_next = exp_to_next_level(level);
  p.move0 = MOVE_TACKLE;
  p.move1 = s->sig_move;

  return p;
}

int exp_to_next_level(int level)
{
  return level * level * level;   /* level^3 exp to advance */
}

/* Level up: recompute stats, keep the HP gained, refresh the exp target. */
static void level_up(pokemon_t *p)
{
  int old_hp = p->hp;
  p->level++;
  compute_stats(p);
  p->cur_hp += p->hp - old_hp;
  if (p->cur_hp > p->hp) p->cur_hp = p->hp;
  p->exp_to_next = exp_to_next_level(p->level);
}

int gain_exp(pokemon_t *p, int amount)
{
  int levels = 0;
  p->exp += amount;
  while (p->level < 100 && p->exp >= p->exp_to_next) {
    p->exp -= p->exp_to_next;
    level_up(p);
    levels++;

    /* Evolution: change species and adopt the new form's signature move. */
    const species_t *s = &pokemon_species[p->species];
    if (s->evolves_to >= 0 && p->level >= s->evolve_level) {
      int old_hp = p->hp;
      p->species = s->evolves_to;
      compute_stats(p);
      p->cur_hp += p->hp - old_hp;
      if (p->cur_hp > p->hp) p->cur_hp = p->hp;
      p->move1 = pokemon_species[p->species].sig_move;
    }
  }
  return levels;
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

/* Type-effectiveness multiplier of a move against a defender's type(s). */
double move_effectiveness(int move_idx, int def_species)
{
  const move_t *m = &pokemon_moves[move_idx];
  const species_t *ds = &pokemon_species[def_species];
  double eff = type_effectiveness(m->type, ds->type1);
  if (ds->type2 != TYPE_NONE) {
    eff *= type_effectiveness(m->type, ds->type2);
  }
  return eff;
}

/* Simplified damage: physical attack stat vs. physical defense, scaled by
 * type effectiveness (the move's type vs. the defender's type(s)). */
int battle_damage(const pokemon_t *atk, const pokemon_t *def, int move_idx)
{
  const move_t *m = &pokemon_moves[move_idx];
  int dmg;

  if (m->power == 0) return 0;

  dmg = ((2 * atk->level / 5 + 2) * m->power * atk->atk / def->def) / 50 + 2;
  dmg = dmg * (85 + rand() % 31) / 100;   /* ±15% variance */

  double eff = move_effectiveness(move_idx, def->species);
  if (eff <= 0.0) return 0;   /* immune */
  dmg = (int)(dmg * eff);

  return dmg < 1 ? 1 : dmg;
}

/* ------------------------------------------------------------------------- *
 * Per-biome encounter tables.  Each walkable terrain has its own species pool *
 * so the world feels regional (mountains -> rock, desert -> ground/fire, ...). *
 * ------------------------------------------------------------------------- */

static const char *const pool_grass[] = {
  "Caterpie", "Weedle", "Oddish", "Bellsprout", "Paras", "Venonat",
  "Rattata", "Pidgey", "Spearow", "Hoothoot", "Sentret", "Ledyba",
  "Spinarak", "Hoppip", "Sunkern", "Mareep", "Pichu", "Aipom",
  "Teddiursa", "Stantler", "Smeargle", "Natu",
};

static const char *const pool_clear[] = {
  "Pidgey", "Spearow", "Rattata", "Meowth", "Sentret", "Furret",
  "Hoothoot", "Noctowl", "Doduo", "Tauros", "Miltank", "Snubbull",
  "Granbull", "Dunsparce", "Girafarig", "Stantler", "Aipom", "Eevee",
};

static const char *const pool_sand[] = {
  "Sandshrew", "Sandslash", "Diglett", "Dugtrio", "Geodude", "Cubone",
  "Marowak", "Growlithe", "Vulpix", "Ponyta", "Ekans", "Phanpy",
  "Donphan", "Larvitar", "Gligar", "Slugma", "Magcargo", "Houndour",
};

static const char *const pool_snow[] = {
  "Seel", "Dewgong", "Shellder", "Cloyster", "Jynx", "Lapras",
  "Swinub", "Piloswine", "Sneasel", "Delibird", "Smoochum",
};

/* Cave-only pool: rock/ground/poison/bug/bat plus a couple of rarities.
 * This is what makes caves worth braving -- these don't spawn elsewhere. */
static const char *const pool_cave[] = {
  "Zubat", "Geodude", "Onix", "Paras", "Clefairy", "Golbat", "Cubone",
  "Diglett", "Scyther", "Larvitar", "Graveler", "Machop", "Mankey",
};

static int pick_from(const char *const *pool, int n)
{
  return species_index(pool[rand() % n]);
}

int encounter_species(char tile)
{
  switch (tile) {
    case ':': return pick_from(pool_grass, (int)(sizeof pool_grass / sizeof *pool_grass));
    case '.': return pick_from(pool_clear, (int)(sizeof pool_clear / sizeof *pool_clear));
    case '=': return pick_from(pool_sand,  (int)(sizeof pool_sand  / sizeof *pool_sand));
    case '*': return pick_from(pool_snow,  (int)(sizeof pool_snow  / sizeof *pool_snow));
    default:  return rand() % pokemon_num_species;
  }
}

int cave_encounter_species(void)
{
  return pick_from(pool_cave, (int)(sizeof pool_cave / sizeof *pool_cave));
}

int encounter_prob(char tile)
{
  switch (tile) {
    case ':': return 10;   /* tall grass */
    case '=': return 8;    /* sand */
    case '*': return 8;    /* snow */
    case '.': return 3;    /* clearing */
    default:  return 0;
  }
}