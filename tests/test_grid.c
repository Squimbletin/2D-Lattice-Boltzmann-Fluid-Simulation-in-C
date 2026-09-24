#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "grid.h"

void test_zero_initialization(void);
void test_no_neighbor_bleed(void);
void test_q_independence(void);
void test_overwrite(void);
void test_invalid_q(void);

int main(void) {
    Grid *grid = create_grid(2, 3, 4);
    assert(grid != NULL);
    assert(grid->rows == 2);
    assert(grid->cols == 3);
    assert(grid->Q == 4);

    set_grid_value(grid, 1, 2, 3, 3.14159);
    assert(fabs(get_grid_value(grid, 1, 2, 3) - 3.14159) < 1e-10);

    assert(fabs(get_grid_value(grid, -1, 0, 0) - 0.0) < 1e-10);
    assert(fabs(get_grid_value(grid, 2, 0, 0) - 0.0) < 1e-10);
    assert(fabs(get_grid_value(grid, 0, 3, 0) - 0.0) < 1e-10);
    assert(fabs(get_grid_value(grid, 0, 0, 4) - 0.0) < 1e-10);

    test_zero_initialization();
    test_no_neighbor_bleed();
    test_q_independence();
    test_overwrite();
    test_invalid_q();

    free_grid(grid);

    printf("test_grid passed\n");
    return 0;
}

// verify calloc zeroed everything
void test_zero_initialization() {
    Grid *g = create_grid(3, 3, 9);
    assert(g != NULL);
    for (int i = 0; i < g->rows; i++)
        for (int j = 0; j < g->cols; j++)
            for (int q = 0; q < g->Q; q++)
                assert(fabs(get_grid_value(g, i, j, q)) < 1e-10);
    free_grid(g);
    printf("PASS: zero initialization\n");
}

// verify values dont bleed into neighboring cells
void test_no_neighbor_bleed() {
    Grid *g = create_grid(3, 3, 9);
    set_grid_value(g, 1, 1, 0, 99.0);
    assert(fabs(get_grid_value(g, 0, 1, 0)) < 1e-10);
    assert(fabs(get_grid_value(g, 2, 1, 0)) < 1e-10);
    assert(fabs(get_grid_value(g, 1, 0, 0)) < 1e-10);
    assert(fabs(get_grid_value(g, 1, 2, 0)) < 1e-10);
    free_grid(g);
    printf("PASS: no neighbor bleed\n");
}

// verify all 9 q directions are independent per cell
void test_q_independence() {
    Grid *g = create_grid(2, 2, 9);
    for (int q = 0; q < 9; q++)
        set_grid_value(g, 0, 0, q, (double)q);
    for (int q = 0; q < 9; q++)
        assert(fabs(get_grid_value(g, 0, 0, q) - (double)q) < 1e-10);
    free_grid(g);
    printf("PASS: q independence\n");
}

// verify overwriting a value works correctly
void test_overwrite() {
    Grid *g = create_grid(2, 2, 9);
    set_grid_value(g, 0, 0, 0, 1.0);
    set_grid_value(g, 0, 0, 0, 2.0);
    assert(fabs(get_grid_value(g, 0, 0, 0) - 2.0) < 1e-10);
    free_grid(g);
    printf("PASS: overwrite\n");
}

// verify bounds checking on q dimension
void test_invalid_q() {
    Grid *g = create_grid(2, 2, 9);
    set_grid_value(g, 0, 0, 9, 99.0);
    assert(fabs(get_grid_value(g, 0, 0, 9)) < 1e-10);
    free_grid(g);
    printf("PASS: invalid q bounds\n");
}
