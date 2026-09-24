#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "grid.h"
#include "lbm.h"
#include <string.h>
#include <assert.h>

void test_equilibrium();
void test_calculate_rho();
void test_calculate_velocity();
void test_collision_step();
void test_streaming_step();

const double w[9] = {
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

int main(void) {
    test_equilibrium();
    test_calculate_rho();
    test_calculate_velocity();
    test_collision_step();
    test_streaming_step();
    return 0;
}

void test_equilibrium() {
    double sum = 0.0;
    for (int q = 0; q < 9; q++) {
        double feq = equilibrium(q, 1.0, 0.0, 0.0);
        assert(fabs(feq - w[q]) < 1e-10); // in equilibrium with zero velocity feq should equal the weight

        feq = equilibrium(q, 1.0, 0.1, 0.0);
        sum += feq;
        printf("feq for q=%d with ux=0.1: %f\n", q, feq);
    }
    assert (fabs(sum - 1.0) < 1e-10); // sum of feq should equal density 
    printf("PASS: equilibrium function\n");
}

void test_calculate_rho() {
    Grid *g = create_grid(3, 3, 9);
    for (int q = 0; q < 9; q++) {
        set_grid_value(g, 1, 1, q, w[q]);
    }
    double rho = calculate_rho(g, 1, 1);
    assert(fabs(rho - 1.0) < 1e-10); // rho should equal the sum of feq
    free_grid(g);
    printf("PASS: calculate_rho function\n");
}

void test_calculate_velocity() {
    Grid *g = create_grid(3, 3, 9);
    for (int q = 0; q < 9; q++) {
        set_grid_value(g, 1, 1, q, w[q]);
    }
    double rho = calculate_rho(g, 1, 1);
    double ux, uy;
    calculate_velocity(g, 1, 1, rho, &ux, &uy);
    assert(fabs(ux) < 1e-10); // velocity should be zero in equilibrium
    assert(fabs(uy) < 1e-10);
    free_grid(g);
    printf("PASS: calculate_velocity function\n");
}

void test_collision_step() {
    Grid *g = create_grid(3, 3, 9);
    for (int q = 0; q < 9; q++) {
        set_grid_value(g, 1, 1, q, w[q] + 0.01); 
    }
    collision_step(g , 1.0); // with tau=1.0 should go to equilibrium
    double rho = calculate_rho(g, 1, 1);
    double ux, uy;
    calculate_velocity(g, 1, 1, rho, &ux, &uy);
    assert(fabs(ux) < 1e-10); // should relax back towards zero velocity
    assert(fabs(uy) < 1e-10);
    free_grid(g);
    printf("PASS: collision_step function\n");
}

void test_streaming_step() {
    Grid *g = create_grid(3, 3, 9);
    set_grid_value(g, 1, 1, 1, 1.0); // set a value in the east direction
    streaming_step(g);
    assert(fabs(get_grid_value(g, 1, 2, 1) - 1.0) < 1e-10); // should have streamed to the east
    free_grid(g);
    printf("PASS: streaming_step function\n");
}
