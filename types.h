#ifndef TYPES_H
#define TYPES_H

/* Generation 1-2 types (17).  Order matches the veekun type_id 1..17. */
enum type_e {
  TYPE_NORMAL = 0,
  TYPE_FIGHTING,
  TYPE_FLYING,
  TYPE_POISON,
  TYPE_GROUND,
  TYPE_ROCK,
  TYPE_BUG,
  TYPE_GHOST,
  TYPE_STEEL,
  TYPE_FIRE,
  TYPE_WATER,
  TYPE_GRASS,
  TYPE_ELECTRIC,
  TYPE_PSYCHIC,
  TYPE_ICE,
  TYPE_DRAGON,
  TYPE_DARK,
  NUM_TYPES,
};

/* Sentinel for a single-typed species' second type. */
#define TYPE_NONE (-1)

extern const char *const type_names[NUM_TYPES];

/* Damage multiplier for an attack of type `atk` against a defender of type
 * `def`: 0.0 immune, 0.5 resisted, 1.0 neutral, 2.0 super effective. */
double type_effectiveness(int atk, int def);

#endif /* TYPES_H */