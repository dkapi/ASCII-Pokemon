#ifndef UI_H
#define UI_H

/* Color pairs, initialized by init_curses() in worldgen.c.  Keep the order in
 * sync with the init_pair() calls there. */
enum {
  CP_WHITE = 1, CP_CYAN, CP_MAGENTA, CP_YELLOW,
  CP_BLUE, CP_GREEN, CP_BROWN, CP_RED, CP_LBLUE,
};

/* Render a 10-segment HP bar ("[######....]") into out (>= 13 bytes). */
static inline void hp_bar(char out[13], int cur, int max)
{
  int filled = (max > 0) ? (cur * 10) / max : 0;
  if (filled < 0)  filled = 0;
  if (filled > 10) filled = 10;
  out[0] = '[';
  for (int i = 0; i < 10; i++) out[1 + i] = (i < filled) ? '#' : '.';
  out[11] = ']';
  out[12] = '\0';
}

/* HP fraction -> color pair: green >=50%, amber >=25%, else red. */
static inline int hp_color(int cur, int max)
{
  int pct;
  if (max <= 0) return CP_RED;
  pct = cur * 100 / max;
  if (pct >= 50) return CP_GREEN;
  if (pct >= 25) return CP_YELLOW;
  return CP_RED;
}

/* Render a 10-segment EXP bar ("[====....]") into out (>= 13 bytes). */
static inline void exp_bar(char out[13], int exp, int exp_to_next)
{
  int filled = (exp_to_next > 0) ? (exp * 10) / exp_to_next : 0;
  if (filled < 0)  filled = 0;
  if (filled > 10) filled = 10;
  out[0] = '[';
  for (int i = 0; i < 10; i++) out[1 + i] = (i < filled) ? '=' : '.';
  out[11] = ']';
  out[12] = '\0';
}

#endif /* UI_H */