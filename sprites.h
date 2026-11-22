#ifndef SPRITES_H
#define SPRITES_H

#include "types.h"

#define SPRITE_H 7
#define SPRITE_W 9

/* One ASCII sprite per type, indexed by TYPE_*.  Each is SPRITE_H rows of
 * SPRITE_W characters. */
extern const char *const type_sprites[NUM_TYPES][SPRITE_H];

#endif /* SPRITES_H */