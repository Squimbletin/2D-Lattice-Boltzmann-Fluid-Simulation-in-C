#ifndef GRID_H
#define GRID_H


//Grid to hold D2Q9 Data 2 primary dimensions (rows and cols) and 1 for the 9 velocity directions (Q)
typedef struct {
    int rows;
    int cols;
    int Q;
    double *data;
} Grid;

Grid *create_grid(int rows, int cols, int Q);
void free_grid(Grid *grid);
double get_grid_value(Grid *grid, int row, int col, int q);
void set_grid_value(Grid *grid, int row, int col, int q, double value);

#endif