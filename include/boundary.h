#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "grid.h"

/* Marks which cells are solid (walls / obstacles). rows = y (row+ is "north"), cols = x. */
typedef struct {
    int rows;
    int cols;
    unsigned char *solid;
} Mask;

Mask *create_mask(int rows, int cols);
void free_mask(Mask *mask);
int is_solid(const Mask *mask, int row, int col);

/* Shape builders for mock barriers (coordinates are in grid cells) */
void mask_add_walls(Mask *mask);                                   /* solid top and bottom rows */
void mask_add_circle(Mask *mask, double cx, double cy, double r);
void mask_add_rect(Mask *mask, double x0, double y0, double x1, double y1);
void mask_add_airfoil(Mask *mask, double x_lead, double y_center, double chord,
                      double thickness_frac, double angle_deg);    /* NACA 00xx, +angle = nose up */

/* Flow setup and per-step boundary conditions */
void init_flow(Grid *grid, double rho, double ux, double uy);
void apply_bounce_back(Grid *grid, const Mask *mask);              /* call after collision, before streaming */
void apply_inlet(Grid *grid, const Mask *mask, double ux);         /* Zou-He fixed-velocity left edge */
void apply_outlet(Grid *grid, const Mask *mask, double rho);       /* Zou-He fixed-density right edge */

#endif
