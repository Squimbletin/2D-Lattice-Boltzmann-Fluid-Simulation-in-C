#include "grid.h"
#include <stdlib.h>

/** @brief Creates a new grid with the specified dimensions 
 * @param rows Number of rows in the grid
 * @param cols Number of columns in the grid
 * @param Q Number of layers in the grid
 * @return Pointer to the created grid or NULL on failure
 */
Grid *create_grid(int rows, int cols, int Q) {
    Grid *grid = (Grid *)calloc(1, sizeof(Grid));
    if (grid == NULL) {
        return NULL; 
    }
    grid->rows = rows;
    grid->cols = cols;
    grid->Q = Q;
    grid->data = (double *)calloc(rows * cols * Q, sizeof(double));
    if (grid->data == NULL) {
        free(grid); 
        return NULL; 
    }
    return grid;
}

/** @brief Frees the memory allocated for the grid
 * @param grid Pointer to the grid to be freed
 */
void free_grid(Grid *grid) {
    if (grid != NULL) {
        free(grid->data);
        free(grid);
    }
}

/** @brief Retrieves the value at the specified position in the grid
 * @param grid Pointer to the grid
 * @param row Row index
 * @param col Column index
 * @param q Layer index
 * @return Value at the specified position or 0.0 if out of bounds
 */
double get_grid_value(Grid *grid, int row, int col, int q) {
    if (grid == NULL || row < 0 || row >= grid->rows || col < 0 || col >= grid->cols || q < 0 || q >= grid->Q) {
        return 0.0; 
    }
    return grid->data[row * grid->cols * grid->Q + col * grid->Q + q];
}

/** @brief Sets the value at the specified position in the grid
 * @param grid Pointer to the grid
 * @param row Row index
 * @param col Column index
 * @param q Layer index
 * @param value Value to set
 */
void set_grid_value(Grid *grid, int row, int col, int q, double value) {
    if (grid == NULL || row < 0 || row >= grid->rows || col < 0 || col >= grid->cols || q < 0 || q >= grid->Q) {
        return; 
    }
    grid->data[row * grid->cols * grid->Q + col * grid->Q + q] = value;
}