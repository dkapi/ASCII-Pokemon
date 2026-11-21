#ifndef NOISE_H
#define NOISE_H

/* Deterministic 2D value noise + fractal Brownian motion, no math library
 * dependency.  Output is in roughly [-1, 1], driven by the input coordinates,
 * so feeding WORLD coordinates makes terrain continuous across map borders. */

double noise2(double x, double y);
double fbm2(double x, double y, int octaves);

#endif /* NOISE_H */