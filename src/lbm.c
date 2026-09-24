#include <stdlib.h>
#include <math.h>
#include "grid.h"
#include <string.h>

// D2Q9 weights
static const double w[9] = {
    4.0/9.0,   // rest
    1.0/9.0,   // east
    1.0/9.0,   // north
    1.0/9.0,   // west
    1.0/9.0,   // south
    1.0/36.0,  // northeast
    1.0/36.0,  // northwest
    1.0/36.0,  // southwest
    1.0/36.0   // southeast
};

static const int ex[9]    = { 0, 1, 0, -1,  0,  1, -1, -1,  1};
static const int ey[9]    = { 0, 0, 1,  0, -1,  1,  1, -1, -1};
static const double cs2 = 1.0/3.0; // speed of sound squared

/**
 * @brief Computes the equilibrium distribution function for a given velocity direction
 * @param q Velocity direction index (0-8 for D2Q9)
 * @param rho Local density
 * @param ux Local velocity in x direction
 * @param uy Local velocity in y direction
 * @return Equilibrium distribution value for the specified direction
 */
double equilibrium(int q, double rho, double ux, double uy){
    double eu = ex[q]*ux + ey[q]*uy; // e dot u
    double uu = ux*ux + uy*uy; // u dot u
    double result = w[q] * rho * (1.0 + eu/cs2 + eu*eu/(2.0*cs2*cs2) - uu/(2.0*cs2));
    return result;
}

/** 
 * @brief Calculates the local density at a given grid cell
 * @param grid Pointer to the grid
 * @param row Row index of the cell
 * @param col Column index of the cell
 * @return Local density at the specified cell
 */
double calculate_rho(Grid *grid, int row, int col) {
    double rho = 0.0;
    for (int q = 0; q < grid->Q; q++) {
        rho += get_grid_value(grid, row, col, q);
    }
    return rho;
}

/** 
 * @brief Calculates the local velocity at a given grid cell
 * @param grid Pointer to the grid
 * @param row Row index of the cell
 * @param col Column index of the cell
 * @param rho Local density at the cell
 * @param ux Pointer to store calculated velocity in x direction
 * @param uy Pointer to store calculated velocity in y direction
 */
void calculate_velocity(Grid *grid, int row, int col, double rho, double *ux, double *uy) {
    if (rho < 1e-10) {
        *ux = 0.0;
        *uy = 0.0;
        return;
    }
    *ux = 0.0;
    *uy = 0.0;
    for (int q = 0; q < grid->Q; q++) {
        *ux += get_grid_value(grid, row, col, q) * ex[q];
        *uy += get_grid_value(grid, row, col, q) * ey[q];
    }
    *ux /= rho;
    *uy /= rho;
}

/** 
 * @brief Performs the collision step of the LBM algorithm on the grid
 * @param grid Pointer to the grid
 * @param tau how fast the system relaxes towards equilibrium (relaxation time)
 */
void collision_step(Grid *grid, double tau) {
    for (int row = 0; row < grid->rows; row++) {
        for (int col = 0; col < grid->cols; col++) {
            double rho = calculate_rho(grid, row, col);
            double ux, uy;
            calculate_velocity(grid, row, col, rho, &ux, &uy);
            for (int q = 0; q < grid->Q; q++) {
                double feq = equilibrium(q, rho, ux, uy);
                double f = get_grid_value(grid, row, col, q);
                double f_post = f - (f - feq) / tau;
                set_grid_value(grid, row, col, q, f_post);
            }
        }
    }
}

/** 
 * @brief Performs the streaming step of the LBM algorithm on the grid
 * @param grid Pointer to the grid
 */
void streaming_step(Grid *grid) {
    Grid temp_grid = *grid;
    temp_grid.data = (double *)malloc(grid->rows * grid->cols * grid->Q * sizeof(double));
    memcpy(temp_grid.data, grid->data, grid->rows * grid->cols * grid->Q * sizeof(double));

    for (int row = 0; row < grid->rows; row++) {
        for (int col = 0; col < grid->cols; col++) {
            for (int q = 0; q < grid->Q; q++) {
                int src_row = (row - ey[q] + grid->rows) % grid->rows;
                int src_col = (col - ex[q] + grid->cols) % grid->cols;
                double f = get_grid_value(&temp_grid, src_row, src_col, q);
                set_grid_value(grid, row, col, q, f);
            }
        }
    }
    free(temp_grid.data);
}