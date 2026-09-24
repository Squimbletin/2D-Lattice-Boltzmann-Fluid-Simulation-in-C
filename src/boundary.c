#include "boundary.h"
#include "lbm.h"
#include <stdlib.h>
#include <math.h>

/* Opposite direction for each D2Q9 index: rest, E, N, W, S, NE, NW, SW, SE */
static const int opposite[9] = {0, 3, 4, 1, 2, 7, 8, 5, 6};

Mask *create_mask(int rows, int cols) {
    Mask *mask = (Mask *)calloc(1, sizeof(Mask));
    if (mask == NULL) return NULL;
    mask->rows = rows;
    mask->cols = cols;
    mask->solid = (unsigned char *)calloc((size_t)rows * cols, 1);
    if (mask->solid == NULL) {
        free(mask);
        return NULL;
    }
    return mask;
}

void free_mask(Mask *mask) {
    if (mask != NULL) {
        free(mask->solid);
        free(mask);
    }
}

int is_solid(const Mask *mask, int row, int col) {
    if (mask == NULL || row < 0 || row >= mask->rows || col < 0 || col >= mask->cols) return 0;
    return mask->solid[row * mask->cols + col];
}

static void set_solid(Mask *mask, int row, int col) {
    if (row < 0 || row >= mask->rows || col < 0 || col >= mask->cols) return;
    mask->solid[row * mask->cols + col] = 1;
}

void mask_add_walls(Mask *mask) {
    for (int col = 0; col < mask->cols; col++) {
        set_solid(mask, 0, col);
        set_solid(mask, mask->rows - 1, col);
    }
}

void mask_add_circle(Mask *mask, double cx, double cy, double r) {
    for (int row = 0; row < mask->rows; row++)
        for (int col = 0; col < mask->cols; col++) {
            double dx = col - cx, dy = row - cy;
            if (dx * dx + dy * dy <= r * r) set_solid(mask, row, col);
        }
}

void mask_add_rect(Mask *mask, double x0, double y0, double x1, double y1) {
    for (int row = 0; row < mask->rows; row++)
        for (int col = 0; col < mask->cols; col++)
            if (col >= x0 && col <= x1 && row >= y0 && row <= y1) set_solid(mask, row, col);
}

void mask_add_airfoil(Mask *mask, double x_lead, double y_center, double chord,
                      double thickness_frac, double angle_deg) {
    double a = angle_deg * M_PI / 180.0;
    double ca = cos(a), sa = sin(a);
    for (int row = 0; row < mask->rows; row++)
        for (int col = 0; col < mask->cols; col++) {
            double dx = col - x_lead, dy = row - y_center;
            /* rotate into the airfoil's own frame (chord along +x) */
            double xl = dx * ca - dy * sa;
            double yl = dx * sa + dy * ca;
            double x = xl / chord;
            if (x < 0.0 || x > 1.0) continue;
            double half_thickness = 5.0 * thickness_frac * chord *
                (0.2969 * sqrt(x) - 0.1260 * x - 0.3516 * x * x + 0.2843 * x * x * x - 0.1036 * x * x * x * x);
            if (fabs(yl) <= half_thickness) set_solid(mask, row, col);
        }
}

void init_flow(Grid *grid, double rho, double ux, double uy) {
    for (int row = 0; row < grid->rows; row++)
        for (int col = 0; col < grid->cols; col++)
            for (int q = 0; q < grid->Q; q++)
                set_grid_value(grid, row, col, q, equilibrium(q, rho, ux, uy));
}

/* Full-way bounce-back: on solid cells, send every population back where it came from */
void apply_bounce_back(Grid *grid, const Mask *mask) {
    for (int row = 0; row < grid->rows; row++)
        for (int col = 0; col < grid->cols; col++) {
            if (!is_solid(mask, row, col)) continue;
            for (int q = 1; q < 9; q++) {
                int o = opposite[q];
                if (q < o) {
                    double a = get_grid_value(grid, row, col, q);
                    double b = get_grid_value(grid, row, col, o);
                    set_grid_value(grid, row, col, q, b);
                    set_grid_value(grid, row, col, o, a);
                }
            }
        }
}

/* Zou-He velocity inlet on the left edge: fixes flow speed ux (uy = 0) and works out the
 * three populations that point into the domain from the ones that are already known. */
void apply_inlet(Grid *grid, const Mask *mask, double ux) {
    for (int row = 0; row < grid->rows; row++) {
        if (is_solid(mask, row, 0)) continue;
        double f0 = get_grid_value(grid, row, 0, 0), f2 = get_grid_value(grid, row, 0, 2);
        double f3 = get_grid_value(grid, row, 0, 3), f4 = get_grid_value(grid, row, 0, 4);
        double f6 = get_grid_value(grid, row, 0, 6), f7 = get_grid_value(grid, row, 0, 7);
        double rho = (f0 + f2 + f4 + 2.0 * (f3 + f6 + f7)) / (1.0 - ux);
        set_grid_value(grid, row, 0, 1, f3 + (2.0 / 3.0) * rho * ux);
        set_grid_value(grid, row, 0, 5, f7 - 0.5 * (f2 - f4) + (1.0 / 6.0) * rho * ux);
        set_grid_value(grid, row, 0, 8, f6 + 0.5 * (f2 - f4) + (1.0 / 6.0) * rho * ux);
    }
}

/* Zou-He pressure outlet on the right edge: fixes density rho and lets the flow speed float. */
void apply_outlet(Grid *grid, const Mask *mask, double rho) {
    int last = grid->cols - 1;
    for (int row = 0; row < grid->rows; row++) {
        if (is_solid(mask, row, last)) continue;
        double f0 = get_grid_value(grid, row, last, 0), f1 = get_grid_value(grid, row, last, 1);
        double f2 = get_grid_value(grid, row, last, 2), f4 = get_grid_value(grid, row, last, 4);
        double f5 = get_grid_value(grid, row, last, 5), f8 = get_grid_value(grid, row, last, 8);
        double ux = -1.0 + (f0 + f2 + f4 + 2.0 * (f1 + f5 + f8)) / rho;
        set_grid_value(grid, row, last, 3, f1 - (2.0 / 3.0) * rho * ux);
        set_grid_value(grid, row, last, 6, f8 + 0.5 * (f4 - f2) - (1.0 / 6.0) * rho * ux);
        set_grid_value(grid, row, last, 7, f5 + 0.5 * (f2 - f4) - (1.0 / 6.0) * rho * ux);
    }
}
