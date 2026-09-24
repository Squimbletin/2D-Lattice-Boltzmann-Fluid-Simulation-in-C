#ifndef LBM_H
#define LBM_H

#include "grid.h"

double equilibrium(int q, double rho, double ux, double uy);
double calculate_rho(Grid *grid, int row, int col);
void calculate_velocity(Grid *grid, int row, int col, double rho, double *ux, double *uy);
void collision_step(Grid *grid, double tau);
void streaming_step(Grid *grid);

#endif