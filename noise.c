#include <stdint.h>

#include "noise.h"

static inline double fade(double t)
{
  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

static inline double lerp(double a, double b, double t)
{
  return a + t * (b - a);
}

/* Truncation-based floor, so we don't need -lm. */
static inline int fast_floor(double x)
{
  int i = (int) x;
  return (x < 0.0 && x != (double) i) ? i - 1 : i;
}

/* Deterministic 2D integer hash -> a value in [-1, 1]. */
static inline double hash_corner(int x, int y)
{
  uint32_t h = (uint32_t) x * 0x9E3779B1u;
  h = (h ^ (uint32_t) y) * 0x85EBCA77u;
  h ^= h >> 13;
  h *= 0xC2B2AE35u;
  h ^= h >> 16;
  return (double) h / 2147483648.0 - 1.0;
}

double noise2(double x, double y)
{
  int xi = fast_floor(x);
  int yi = fast_floor(y);
  double xf = x - (double) xi;
  double yf = y - (double) yi;
  double u = fade(xf);
  double v = fade(yf);

  double a = hash_corner(xi,     yi);
  double b = hash_corner(xi + 1, yi);
  double c = hash_corner(xi,     yi + 1);
  double d = hash_corner(xi + 1, yi + 1);

  return lerp(lerp(a, b, u), lerp(c, d, u), v);
}

double fbm2(double x, double y, int octaves)
{
  double total = 0.0;
  double amp = 1.0;
  double freq = 1.0;
  double norm = 0.0;

  for (int i = 0; i < octaves; i++) {
    total += amp * noise2(x * freq, y * freq);
    norm  += amp;
    amp   *= 0.5;
    freq  *= 2.0;
  }
  return total / norm;
}