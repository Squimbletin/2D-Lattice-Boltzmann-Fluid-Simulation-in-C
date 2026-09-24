#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "grid.h"
#include "lbm.h"
#include "boundary.h"
#include "visualize.h"

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(p) _mkdir(p)
#else
#define MAKE_DIR(p) mkdir(p, 0755)
#endif

#define ROWS 140
#define COLS 440
#define U_IN 0.10   /* default inlet speed in lattice units (keep well below 0.3 for stability) */
#define TAU  0.56   /* default relaxation time: viscosity = (tau - 0.5) / 3 */

static void usage(const char *prog) {
    printf("Usage: %s [scene] [steps] [frame_interval] [mode]\n", prog);
    printf("  scene:  cylinder | block | airfoil | barriers   (default: cylinder)\n");
    printf("  steps:  total time steps                        (default: 10000)\n");
    printf("  frame_interval: steps between saved frames      (default: 100)\n");
    printf("  mode:   vorticity | speed                       (default: vorticity)\n");
}

/* Returns 0 on success. Scenes with sharp corners / narrow gaps get more viscosity and a slower inlet to stay stable. */
static int build_scene(Mask *mask, const char *scene, double *tau, double *u_in) {
    *tau = TAU;
    *u_in = U_IN;
    mask_add_walls(mask);
    double mid = ROWS / 2.0;
    if (strcmp(scene, "cylinder") == 0) {
        mask_add_circle(mask, 100, mid + 2, 14);        /* slight offset helps vortex shedding start */
    } else if (strcmp(scene, "block") == 0) {
        mask_add_rect(mask, 90, mid - 12, 116, mid + 14);
    } else if (strcmp(scene, "airfoil") == 0) {
        mask_add_airfoil(mask, 70, mid + 4, 90, 0.12, 12.0);   /* 12% thick, 12 degrees nose-up */
    } else if (strcmp(scene, "barriers") == 0) {
        *tau = 0.62;
        *u_in = 0.06;
        mask_add_circle(mask, 90, mid + 24, 9);
        mask_add_circle(mask, 90, mid - 24, 9);
        mask_add_rect(mask, 175, mid + 4, 182, mid + 40);       /* plate hanging from above */
        mask_add_rect(mask, 255, mid - 40, 262, mid - 4);       /* plate rising from below */
    } else {
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        usage(argv[0]);
        return 0;
    }
    const char *scene = argc > 1 ? argv[1] : "cylinder";
    int steps = argc > 2 ? atoi(argv[2]) : 10000;
    int interval = argc > 3 ? atoi(argv[3]) : 100;
    VisMode mode = (argc > 4 && strcmp(argv[4], "speed") == 0) ? VIS_SPEED : VIS_VORTICITY;
    if (steps <= 0 || interval <= 0) { usage(argv[0]); return 1; }

    Grid *grid = create_grid(ROWS, COLS, 9);
    Mask *mask = create_mask(ROWS, COLS);
    if (grid == NULL || mask == NULL) { fprintf(stderr, "Out of memory\n"); return 1; }
    double tau = TAU, u_in = U_IN;
    if (build_scene(mask, scene, &tau, &u_in) != 0) {
        fprintf(stderr, "Unknown scene '%s'\n", scene);
        usage(argv[0]);
        return 1;
    }

    MAKE_DIR("frames");
    init_flow(grid, 1.0, u_in, 0.0);

    printf("Scene: %s | %dx%d grid | %d steps | frame every %d\n", scene, COLS, ROWS, steps, interval);
    int frame = 0;
    for (int step = 0; step <= steps; step++) {
        if (step % interval == 0) {
            char path[64];
            snprintf(path, sizeof(path), "frames/frame_%04d.ppm", frame++);
            if (write_frame(path, grid, mask, 2, mode, u_in) != 0) {
                fprintf(stderr, "Could not write %s (does the frames/ folder exist?)\n", path);
                return 1;
            }
            double vmax = max_speed(grid, mask);
            printf("step %6d | max speed %.4f | %s\n", step, vmax, path);
            if (vmax < 0 || vmax > 0.4) {
                fprintf(stderr, "Simulation became unstable - try a larger tau or smaller U_IN\n");
                return 1;
            }
        }
        collision_step(grid, tau);
        apply_bounce_back(grid, mask);
        streaming_step(grid);
        apply_inlet(grid, mask, u_in);
        apply_outlet(grid, mask, 1.0);
    }

    printf("Done: %d frames in frames/. Make an animation with: python tools/make_gif.py\n", frame);
    free_grid(grid);
    free_mask(mask);
    return 0;
}
