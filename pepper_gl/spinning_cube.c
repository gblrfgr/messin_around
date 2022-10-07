#include "pepper_gl.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

// in units of radians
#define YAW 0.45
#define PITCH 2.6
#define ROLL 0.9

#define SCREEN_HEIGHT 40
#define SCREEN_WIDTH 80

int main() {
    const pgl_vector3_t CUBE_POINTS_INITIAL[8] = {
        {1, 1, 1}, {1, 1, -1}, {1, -1, 1}, {1, -1, -1}, {-1, 1, 1}, {-1, 1, -1}, {-1, -1, 1}, {-1, -1, -1},
    };

    const unsigned int CUBE_EDGES[12][2] = {{0, 1}, {0, 2}, {0, 4}, {7, 6}, {7, 5}, {7, 3},
                                            {1, 3}, {3, 2}, {2, 6}, {6, 4}, {4, 5}, {5, 1}};

    pgl_camera_t cam = {
        M_PI_2,
        {0, 0, -3},
        {0, 0, 1},
        {1, 0, 0},
    };

    char screen_data[SCREEN_WIDTH][SCREEN_HEIGHT];
    pgl_screen_t screen = {SCREEN_WIDTH, SCREEN_HEIGHT, (char*)screen_data};
    pgl_vector2_t projected_points[8];
    while (true) {
        pgl_screen_clear(&screen, ' ');
        double scale = (double)clock() / CLOCKS_PER_SEC;
        pgl_matrix33_t rotation_matrix = pgl_gen_rotation_matrix(YAW * scale, PITCH * scale, ROLL * scale);
        for (unsigned int i = 0; i < 8; i++) {
            pgl_project_2d(cam, pgl_apply_matrix33(rotation_matrix, CUBE_POINTS_INITIAL[i]), &projected_points[i]);
        }
        for (unsigned int i = 0; i < 12; i++) {
            pgl_render_line(&screen, projected_points[CUBE_EDGES[i][0]], projected_points[CUBE_EDGES[i][1]], 'O');
        }
        pgl_draw_screen(screen, stdout);
    }

    return 0;
}