#include "visualize.h"
#include "lbm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct { double r, g, b; } Color;

static Color lerp(Color a, Color b, double t) {
    Color c = { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t };
    return c;
}

/* t in [-1, 1]: blue (spinning one way) -> white (still) -> red (spinning the other way) */
static Color diverging(double t) {
    if (t > 1.0) t = 1.0;
    if (t < -1.0) t = -1.0;
    Color blue = {30, 90, 210}, white = {245, 245, 245}, red = {215, 45, 40};
    return t < 0 ? lerp(white, blue, -t) : lerp(white, red, t);
}

/* t in [0, 1]: dark -> purple -> orange -> yellow */
static Color heat(double t) {
    if (t > 1.0) t = 1.0;
    if (t < 0.0) t = 0.0;
    Color stops[4] = { {10, 10, 40}, {120, 30, 130}, {235, 110, 30}, {252, 235, 110} };
    double s = t * 3.0;
    int i = (int)s;
    if (i >= 3) i = 2;
    return lerp(stops[i], stops[i + 1], s - i);
}

static void cell_velocity(Grid *grid, int row, int col, double *ux, double *uy) {
    double rho = calculate_rho(grid, row, col);
    calculate_velocity(grid, row, col, rho, ux, uy);
}

double max_speed(Grid *grid, const Mask *mask) {
    double best = 0.0;
    for (int row = 0; row < grid->rows; row++)
        for (int col = 0; col < grid->cols; col++) {
            if (is_solid(mask, row, col)) continue;
            double ux, uy;
            cell_velocity(grid, row, col, &ux, &uy);
            double s = sqrt(ux * ux + uy * uy);
            if (isnan(s)) return -1.0;
            if (s > best) best = s;
        }
    return best;
}

int write_frame(const char *path, Grid *grid, const Mask *mask, int scale, VisMode mode, double u_ref) {
    int rows = grid->rows, cols = grid->cols;
    double *ux = (double *)malloc(sizeof(double) * rows * cols);
    double *uy = (double *)malloc(sizeof(double) * rows * cols);
    if (ux == NULL || uy == NULL) { free(ux); free(uy); return -1; }

    for (int row = 0; row < rows; row++)
        for (int col = 0; col < cols; col++)
            cell_velocity(grid, row, col, &ux[row * cols + col], &uy[row * cols + col]);

    FILE *fp = fopen(path, "wb");
    if (fp == NULL) { free(ux); free(uy); return -1; }
    fprintf(fp, "P6\n%d %d\n255\n", cols * scale, rows * scale);

    double vort_range = 0.12 * u_ref;   /* vorticity that maps to full colour (tuned for ~30-cell obstacles) */
    for (int img_row = rows - 1; img_row >= 0; img_row--) {   /* flip so "north" is up */
        unsigned char *line = (unsigned char *)malloc((size_t)cols * scale * 3);
        for (int col = 0; col < cols; col++) {
            Color c;
            if (is_solid(mask, img_row, col)) {
                c.r = c.g = c.b = 55;
            } else if (mode == VIS_SPEED) {
                double s = sqrt(ux[img_row * cols + col] * ux[img_row * cols + col] +
                                uy[img_row * cols + col] * uy[img_row * cols + col]);
                c = heat(s / (1.6 * u_ref));
            } else {
                /* vorticity = d(uy)/dx - d(ux)/dy using central differences; treat solid neighbours as at rest */
                int cl = col > 0 ? col - 1 : col, cr = col < cols - 1 ? col + 1 : col;
                int rd = img_row > 0 ? img_row - 1 : img_row, ru = img_row < rows - 1 ? img_row + 1 : img_row;
                double uy_r = is_solid(mask, img_row, cr) ? 0.0 : uy[img_row * cols + cr];
                double uy_l = is_solid(mask, img_row, cl) ? 0.0 : uy[img_row * cols + cl];
                double ux_u = is_solid(mask, ru, col) ? 0.0 : ux[ru * cols + col];
                double ux_d = is_solid(mask, rd, col) ? 0.0 : ux[rd * cols + col];
                double w = ((uy_r - uy_l) / (double)(cr - cl ? cr - cl : 1)) -
                           ((ux_u - ux_d) / (double)(ru - rd ? ru - rd : 1));
                c = diverging(w / vort_range);
            }
            for (int s = 0; s < scale; s++) {
                int x = (col * scale + s) * 3;
                line[x] = (unsigned char)c.r;
                line[x + 1] = (unsigned char)c.g;
                line[x + 2] = (unsigned char)c.b;
            }
        }
        for (int s = 0; s < scale; s++) fwrite(line, 1, (size_t)cols * scale * 3, fp);
        free(line);
    }
    fclose(fp);
    free(ux);
    free(uy);
    return 0;
}
