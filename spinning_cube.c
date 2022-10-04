#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// in units of radians
#define YAW 0.45
#define PITCH 2.6
#define ROLL 0.9

#define SCREEN_HEIGHT 40
#define SCREEN_WIDTH 80

typedef struct matrix_t {
    const unsigned int width;
    const unsigned int height;
    double* buf;
} matrix_t;

typedef struct vector_t {
    const unsigned int dimensions;
    double* buf;
} vector_t;

typedef struct camera_t {
    double fov;
    vector_t position;
    vector_t forward;
    vector_t right;
} camera_t;

typedef struct screen_t {
    unsigned int width;
    unsigned int height;
    char* buf;
} screen_t;

void matrix_scale(matrix_t* mat, double k) {
    for (unsigned int y = 0; y < mat->height; y++) {
        for (unsigned int x = 0; x < mat->width; x++) {
            mat->buf[x + y * mat->width] *= k;
        }
    }
}

void vector_scale(vector_t* vec, double k) {
    for (unsigned int i = 0; i < vec->dimensions; i++) {
        vec->buf[i] *= k;
    }
}

void vector_subtract(vector_t* a, vector_t b) {
    // subtracts b from a and puts result in a

    assert(a->dimensions == b.dimensions);
    for (unsigned int i = 0; i < a->dimensions; i++) {
        a->buf[i] -= b.buf[i];
    }
}

double vector_dot(vector_t a, vector_t b) {
    assert(a.dimensions == b.dimensions);
    double acc = 0.0;
    for (unsigned int i = 0; i < a.dimensions; i++) {
        acc += a.buf[i] * b.buf[i];
    }
    return acc;
}

void apply_matrix(matrix_t mat, vector_t vec, vector_t* out) {
    assert(mat.width == vec.dimensions);
    assert(mat.height == out->dimensions);

    for (unsigned int i = 0; i < out->dimensions; i++) {
        double acc = 0;
        for (unsigned int j = 0; j < vec.dimensions; j++) {
            acc += vec.buf[j] * mat.buf[j + i * mat.width];
        }
        out->buf[i] = acc;
    }
}

void matrix_multiply(matrix_t a, matrix_t b, matrix_t* out) {
    assert(a.height == b.width);
    assert(a.height == out->height);
    assert(b.width == out->width);

    for (unsigned int y = 0; y < out->height; y++) {
        for (unsigned int x = 0; x < out->width; x++) {
            double acc = 0.0;
            for (unsigned int i = 0; i < a.width; i++) {
                acc += a.buf[y * a.width + i] * b.buf[i * b.width + x];
            }
            out->buf[y * out->width + x] = acc;
        }
    }
}

void matrix_pprint(matrix_t mat) {
    for (unsigned int y = 0; y < mat.height; y++) {
        printf("[\t");
        for (unsigned int x = 0; x < mat.width; x++) {
            printf("%g\t", mat.buf[x + y * mat.width]);
        }
        printf("]\n");
    }
}

void vector_pprint(vector_t vec) {
    for (unsigned int i = 0; i < vec.dimensions; i++) {
        printf("[\t%g\t]\n", vec.buf[i]);
    }
}

void vector_copy(vector_t in, vector_t* out) {
    assert(in.dimensions == out->dimensions);
    for (unsigned int i = 0; i < in.dimensions; i++) {
        out->buf[i] = in.buf[i];
    }
}

void matrix_copy(matrix_t in, matrix_t* out) {
    assert(in.height == out->height);
    assert(in.width == out->width);
    for (unsigned int y = 0; y < in.height; y++) {
        for (unsigned int x = 0; x < in.width; x++) {
            out->buf[y * in.width + x] = in.buf[y * in.width + x];
        }
    }
}

double vector_magnitude(vector_t vec) {
    double acc = 0.0;
    for (unsigned int i = 0; i < vec.dimensions; i++) {
        acc += vec.buf[i] * vec.buf[i];
    }
    return sqrt(acc);
}

void gen_rotation_matrix(double yaw, double pitch, double roll, matrix_t* out) {
    // generates separate matrices for yaw, pitch, and roll and multiplies them together
    // more computation than required? yes. conceptually really simple? also yes.

    assert(out->width == out->height && out->height == 3);

    // clang-format off
    double roll_matrix_data[3][3] = {
        {cos(roll), -sin(roll), 0},
        {sin(roll), cos(roll), 0},
        {0, 0, 1},
    };
    double pitch_matrix_data[3][3] = {
        {1, 0, 0},
        {0, sin(pitch), cos(pitch)},
        {0, cos(pitch), -sin(pitch)},
    };
    double yaw_matrix_data[3][3] = {
        {cos(yaw), 0, -sin(yaw)},
        {0, 1, 0},
        {sin(yaw), 0, cos(yaw)}
    };
    double temp_matrix_data[3][3];
    // clang-format on

    matrix_t roll_matrix = {
        3,
        3,
        (double*)roll_matrix_data,
    };
    matrix_t pitch_matrix = {
        3,
        3,
        (double*)pitch_matrix_data,
    };
    matrix_t yaw_matrix = {
        3,
        3,
        (double*)yaw_matrix_data,
    };
    matrix_t temp_matrix = {
        3,
        3,
        (double*)temp_matrix_data,
    };

    matrix_multiply(roll_matrix, pitch_matrix, &temp_matrix);
    matrix_multiply(temp_matrix, yaw_matrix, out);
}

bool project_2d(camera_t cam, vector_t in, vector_t* out) {
    // return value is whether or not projection was successful
    // out has x and y in the range from -1 to 1

    assert(in.dimensions == 3);
    assert(out->dimensions == 2);

    double rebased_data[3];
    vector_t rebased = {3, rebased_data};
    vector_copy(in, &rebased);
    vector_subtract(&rebased, cam.position);
    vector_scale(&rebased, 1 / vector_magnitude(rebased));

    // check if within view
    // assumes cam.forward is always normalized, as it should be
    if (acos(vector_dot(cam.forward, rebased)) > cam.fov / 2.0) {
        return false;
    }

    // flatten this baby onto a screen
    double offset_data[3];
    vector_t offset = {3, offset_data};
    vector_copy(cam.forward, &offset);
    vector_scale(&offset, vector_dot(cam.forward, rebased));
    vector_subtract(&rebased, offset);
    double r = vector_magnitude(rebased);
    vector_scale(&rebased, 1 / r);
    double theta = acos(vector_dot(cam.right, rebased));

    // TIME TO COMPUTE A CROSS PRODUCT YAYYYYY
    // since rebased is now coplanar with cam.right, cross(rebased, cam.right) is equal to cam.forward multiplied by a
    // constant! if that constant is negative, it means that theta needs to be negative.
    double crossed_x = cam.right.buf[1] * rebased.buf[2] - cam.right.buf[2] * rebased.buf[1];
    double crossed_y = cam.right.buf[0] * rebased.buf[2] - cam.right.buf[2] * rebased.buf[0];
    double crossed_z = cam.right.buf[0] * rebased.buf[1] - cam.right.buf[1] * rebased.buf[0];
    if ((crossed_x * cam.forward.buf[0] < 0) || (crossed_y * cam.forward.buf[1] < 0) ||
        (crossed_z * cam.forward.buf[2] < 0)) {
        theta *= -1;
    }

    // FINALLY STORE OUT THE RESULT
    out->buf[0] = r * cos(theta);
    out->buf[1] = r * sin(theta);

    return true;
}

void screen_clear(screen_t* s, char color) {
    for (unsigned int i = 0; i < s->width * s->height; i++) {
        s->buf[i] = color;
    }
}

void screen_render(screen_t s) {
    printf("\033[H\033[2J"); // clear screen and pointer to home position
    for (unsigned int line = 0; line < s.height; line++) {
        printf("%.*s\n", s.width, s.buf + line * s.width);
    }
}

void screen_drawline(screen_t* s, vector_t a, vector_t b, char color) {
    // assumes the axes of a and b go from -1 to 1
    // -1 is left/top, 1 is right/bottom

    assert(a.dimensions == 2);
    assert(b.dimensions == 2);

    // determine how many steps we need for a full (not skipping) line
    unsigned int steps = (unsigned int)ceil(fabs(a.buf[0] - b.buf[0]) / 2 * s->width);
    if ((unsigned int)ceil(fabs(a.buf[1] - b.buf[1]) / 2 * s->height) > steps) {
        steps = (unsigned int)ceil(fabs(a.buf[1] - b.buf[1]) / 2 * s->height);
    }

    // step along the line and color in the screen
    double m = (b.buf[1] - a.buf[1]) / (b.buf[0] - a.buf[0]);
    double current_x = a.buf[0];
    double current_y = a.buf[1];
    for (unsigned int i = 0; i < steps; i++) {
        unsigned int transformed_x = (unsigned int)floor(s->width * (current_x + 1) / 2);
        unsigned int transformed_y = (unsigned int)floor(s->height * (current_y + 1) / 2);
        s->buf[transformed_x + transformed_y * s->width] = color;
        current_x = a.buf[0] + (b.buf[0] - a.buf[0]) * ((double)i / steps);
        current_y = a.buf[1] + m * (current_x - a.buf[0]);
    }
}

int main() {
    double rotation_matrix_data[3][3];
    matrix_t rotation_matrix = {
        3,
        3,
        (double*)rotation_matrix_data,
    };

    double cube_points_data[8][3] = {
        {1, 1, 1}, {1, 1, -1}, {1, -1, 1}, {1, -1, -1}, {-1, 1, 1}, {-1, 1, -1}, {-1, -1, 1}, {-1, -1, -1},
    };
    vector_t cube_points[8] = {
        {3, cube_points_data[0]}, {3, cube_points_data[1]}, {3, cube_points_data[2]}, {3, cube_points_data[3]},
        {3, cube_points_data[4]}, {3, cube_points_data[5]}, {3, cube_points_data[6]}, {3, cube_points_data[7]},
    };

    unsigned int cube_edges[12][2] = {{0, 1}, {0, 2}, {0, 4}, {7, 6}, {7, 5}, {7, 3},
                                      {1, 3}, {3, 2}, {2, 6}, {6, 4}, {4, 5}, {5, 1}};

    double rotated_points_data[8][3];
    vector_t rotated_points[8] = {
        {3, rotated_points_data[0]}, {3, rotated_points_data[1]}, {3, rotated_points_data[2]},
        {3, rotated_points_data[3]}, {3, rotated_points_data[4]}, {3, rotated_points_data[5]},
        {3, rotated_points_data[6]}, {3, rotated_points_data[7]},
    };

    double projected_points_data[8][2];
    vector_t projected_points[8] = {
        {2, projected_points_data[0]}, {2, projected_points_data[1]}, {2, projected_points_data[2]},
        {2, projected_points_data[3]}, {2, projected_points_data[4]}, {2, projected_points_data[5]},
        {2, projected_points_data[6]}, {2, projected_points_data[7]},
    };

    double cam_pos_data[3] = {0, 0, -4};
    double cam_fwd_data[3] = {0, 0, 1};
    double cam_rgt_data[3] = {1, 0, 0};
    camera_t cam = {
        M_PI_2,
        {3, cam_pos_data},
        {3, cam_fwd_data},
        {3, cam_rgt_data},
    };

    char screen_data[SCREEN_WIDTH][SCREEN_HEIGHT];
    screen_t screen = {SCREEN_WIDTH, SCREEN_HEIGHT, (char*)screen_data};
    while (true) {
        screen_clear(&screen, ' ');
        double scale = (double)clock() / CLOCKS_PER_SEC;
        gen_rotation_matrix(YAW * scale, PITCH * scale, ROLL * scale, &rotation_matrix);
        for (unsigned int i = 0; i < 8; i++) {
            apply_matrix(rotation_matrix, cube_points[i], &rotated_points[i]);
            project_2d(cam, rotated_points[i], &projected_points[i]);
        }
        for (unsigned int i = 0; i < 12; i++) {
            screen_drawline(&screen, projected_points[cube_edges[i][0]], projected_points[cube_edges[i][1]], 'O');
        }
        screen_render(screen);
    }

    return 0;
}