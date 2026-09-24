#ifndef VISUALIZE_H
#define VISUALIZE_H

#include "grid.h"
#include "boundary.h"

typedef enum { VIS_VORTICITY, VIS_SPEED } VisMode;

/* Writes the current flow field as a PPM image (scale = pixels per cell).
 * Returns 0 on success, -1 on failure. */
int write_frame(const char *path, Grid *grid, const Mask *mask, int scale, VisMode mode, double u_ref);

/* Largest fluid speed in the domain, or -1 if any value is NaN (handy for spotting blow-ups) */
double max_speed(Grid *grid, const Mask *mask);

#endif
