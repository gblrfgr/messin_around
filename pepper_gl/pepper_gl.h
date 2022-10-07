#ifndef PEPPER_GL_H
#define PEPPER_GL_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* errors */

#define PGL_NO_ERROR 0
#define PGL_DYNAMIC_ALLOCATION_FAILURE 1

typedef unsigned int pgl_error_t;

/* pgl_vector2_t and associated operations */

typedef struct pgl_vector2_t {
    double x;
    double y;
} pgl_vector2_t;

pgl_vector2_t pgl_vector2_scale(pgl_vector2_t vec, double k) {
    pgl_vector2_t res = {
        vec.x * k,
        vec.y * k,
    };
    return res;
}

pgl_vector2_t pgl_vector2_add(pgl_vector2_t a, pgl_vector2_t b) {
    pgl_vector2_t res = {
        a.x + b.x,
        a.y + b.y,
    };
    return res;
}

double pgl_vector2_dot(pgl_vector2_t a, pgl_vector2_t b) {
    double acc = 0.0;
    acc += a.x * b.x;
    acc += a.y * b.y;
    return acc;
}

double pgl_vector2_magnitude(pgl_vector2_t vec) { return sqrt(vec.x * vec.x + vec.y * vec.y); }

void pgl_vector2_pprint(pgl_vector2_t vec) { printf("[\t%g\t]\n[\t%g\t]\n", vec.x, vec.y); }

/* pgl_vector3_t and associated operations */

typedef struct pgl_vector3_t {
    double x;
    double y;
    double z;
} pgl_vector3_t;

pgl_vector3_t pgl_vector3_scale(pgl_vector3_t vec, double k) {
    pgl_vector3_t res = {
        vec.x * k,
        vec.y * k,
        vec.z * k,
    };
    return res;
}

pgl_vector3_t pgl_vector3_add(pgl_vector3_t a, pgl_vector3_t b) {
    pgl_vector3_t res = {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
    return res;
}

double pgl_vector3_dot(pgl_vector3_t a, pgl_vector3_t b) {
    double acc = 0.0;
    acc += a.x * b.x;
    acc += a.y * b.y;
    acc += a.z * b.z;
    return acc;
}

pgl_vector3_t pgl_vector3_cross(pgl_vector3_t a, pgl_vector3_t b) {
    pgl_vector3_t res = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
    return res;
}

double pgl_vector3_magnitude(pgl_vector3_t vec) { return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z); }

void pgl_vector3_pprint(pgl_vector3_t vec) { printf("[\t%g\t]\n[\t%g\t]\n[\t%g\t]\n", vec.x, vec.y, vec.z); }

/* pgl_matrix33_t and associated operations */

typedef struct pgl_matrix33_t {
    pgl_vector3_t i;
    pgl_vector3_t j;
    pgl_vector3_t k;
} pgl_matrix33_t;

pgl_vector3_t pgl_apply_matrix33(pgl_matrix33_t mat, pgl_vector3_t vec) {
    pgl_vector3_t res = {0.0, 0.0, 0.0};
    res = pgl_vector3_add(res, pgl_vector3_scale(mat.i, vec.x));
    res = pgl_vector3_add(res, pgl_vector3_scale(mat.j, vec.y));
    res = pgl_vector3_add(res, pgl_vector3_scale(mat.k, vec.z));
    return res;
}

pgl_matrix33_t pgl_matrix33_multiply(pgl_matrix33_t a, pgl_matrix33_t b) {
    pgl_matrix33_t res;
    res.i = pgl_apply_matrix33(b, a.i);
    res.j = pgl_apply_matrix33(b, a.j);
    res.k = pgl_apply_matrix33(b, a.k);
    return res;
}

void pgl_matrix33_pprint(pgl_matrix33_t mat) {
    printf("[\t%g\t%g\t%g\t]\n[\t%g\t%g\t%g\t]\n[\t%g\t%g\t%g\t]\n", mat.i.x, mat.j.x, mat.k.x, mat.i.y, mat.j.y,
           mat.k.y, mat.i.z, mat.j.z, mat.k.z);
}

/* pgl_camera_t and associated operations */

typedef struct pgl_camera_t {
    double fov;
    pgl_vector3_t position;
    pgl_vector3_t forward;
    pgl_vector3_t right;
} pgl_camera_t;

bool pgl_project_2d(pgl_camera_t cam, pgl_vector3_t in, pgl_vector2_t* out) {
    // return value is whether or not point is in view of camera
    // out has x and y in the range from -1 to 1

    in = pgl_vector3_add(in, pgl_vector3_scale(cam.position, -1.0));
    pgl_vector3_t camera_space = {
        pgl_vector3_dot(cam.right, in),
        pgl_vector3_dot(pgl_vector3_cross(cam.right, cam.forward), in),
        pgl_vector3_dot(cam.forward, in),
    };

    out->x = camera_space.x / (camera_space.z * tan(cam.fov / 2.0));
    out->y = camera_space.y / (camera_space.z * tan(cam.fov / 2.0));

    return (-1 <= out->x && out->x <= 1) && (-1 <= out->y && out->y <= 1);
}

/* pgl_screen_t and its associated operations */

typedef struct pgl_screen_t {
    size_t width;
    size_t height;
    char* buf;
} pgl_screen_t;

void pgl_screen_clear(pgl_screen_t* s, char color) {
    for (size_t i = 0; i < s->width * s->height; i++) {
        s->buf[i] = color;
    }
}

void pgl_draw_screen(pgl_screen_t s, FILE* out) {
    if (out == stdout) {
        fprintf(out, "\033[H\033[2J"); // clear screen and pointer to home position
    }
    for (size_t line = 0; line < s.height; line++) {
        fprintf(out, "%.*s\n", (int)s.width, s.buf + line * s.width);
    }
    fflush(out);
}

void pgl_render_line(pgl_screen_t* s, pgl_vector2_t a, pgl_vector2_t b, char color) {
    // assumes the axes of a and b go from -1 to 1
    // -1 is left/top, 1 is right/bottom

    // determine how many steps we need for a full (not skipping) line
    size_t steps = (size_t)ceil(fabs(a.x - b.x) / 2 * s->width);
    if ((size_t)ceil(fabs(a.y - b.y) / 2 * s->height) > steps) {
        steps = (size_t)ceil(fabs(a.y - b.y) / 2 * s->height);
    }

    // step along the line and color in the screen
    double m = (b.y - a.y) / (b.x - a.x);
    double current_x = a.x;
    double current_y = a.y;
    for (size_t i = 0; i < steps; i++) {
        size_t transformed_x = (size_t)floor(s->width * (current_x + 1) / 2);
        size_t transformed_y = (size_t)floor(s->height * (current_y + 1) / 2);
        s->buf[transformed_x + transformed_y * s->width] = color;
        current_x = a.x + (b.x - a.x) * ((double)i / steps);
        current_y = a.y + m * (current_x - a.x);
    }
}

/* pgl_triangle_t, pgl_line_t, and associated operations */

typedef enum pgl_geometry_type_t {
    PGL_TRIANGLE,
    PGL_LINE,
} pgl_geometry_type_t;

typedef struct pgl_triangle_t {
    pgl_vector3_t a;
    pgl_vector3_t b;
    pgl_vector3_t c;
} pgl_triangle_t;

typedef struct pgl_line_t {
    pgl_vector3_t a;
    pgl_vector3_t b;
} pgl_line_t;

/* pgl_renderschedule_t, pgl_renderschedule_entry_t, and associated operations */

#define PGL_RS_INITIAL_LENGTH 10

typedef struct pgl_renderschedule_entry_t {
    union {
        pgl_triangle_t triangle;
        pgl_line_t line;
    };
    pgl_geometry_type_t type;
    char color;
} pgl_renderschedule_entry_t;

typedef struct pgl_renderschedule_t {
    size_t length;
    size_t allocated;
    pgl_renderschedule_entry_t* buf;
} pgl_renderschedule_t;

pgl_error_t pgl_init_renderschedule(pgl_renderschedule_t* sched) {
    // dynamically allocates memory that must be freed with pgl_destroy_renderschedule
    sched->length = 0;
    sched->allocated = PGL_RS_INITIAL_LENGTH;
    sched->buf = (pgl_renderschedule_entry_t*)malloc(sizeof(pgl_renderschedule_entry_t) * PGL_RS_INITIAL_LENGTH);
    if (sched->buf == NULL) {
        return PGL_DYNAMIC_ALLOCATION_FAILURE;
    } else {
        return PGL_NO_ERROR;
    }
}

void pgl_destroy_renderschedule(pgl_renderschedule_t* sched) {
    sched->length = 0;
    sched->allocated = 0;
    free(sched->buf);
    sched->buf = NULL; // salt the earth, prevent future errors
}

pgl_error_t pgl_expand_renderschedule(pgl_renderschedule_t* sched) {
    sched->allocated *= 2;
    sched->buf = (pgl_renderschedule_entry_t*)realloc(sched->buf, sched->allocated);
    if (sched->buf == NULL) {
        return PGL_DYNAMIC_ALLOCATION_FAILURE;
    } else {
        return PGL_NO_ERROR;
    }
}

pgl_error_t pgl_schedule_triangle(pgl_renderschedule_t* sched, pgl_triangle_t t, char color) {
    if (sched->length == sched->allocated) {
        pgl_error_t err = pgl_expand_renderschedule(sched);
        if (err != PGL_NO_ERROR) {
            return err;
        }
    }
    sched->buf[sched->length] = (pgl_renderschedule_entry_t){
        .triangle = t,
        .type = PGL_TRIANGLE,
        .color = color,
    };
    sched->length++;
    return PGL_NO_ERROR;
}

pgl_error_t pgl_schedule_line(pgl_renderschedule_t* sched, pgl_line_t l, char color) {
    if (sched->length == sched->allocated) {
        pgl_error_t err = pgl_expand_renderschedule(sched);
        if (err != PGL_NO_ERROR) {
            return err;
        }
    }
    sched->buf[sched->length] = (pgl_renderschedule_entry_t){
        .line = l,
        .type = PGL_LINE,
        .color = color,
    };
    sched->length++;
    return PGL_NO_ERROR;
}

void pgl_submit_renderschedule(pgl_screen_t* s);

/* convenience functions */

pgl_matrix33_t pgl_gen_rotation_matrix(double yaw, double pitch, double roll) {
    // yaw, pitch, roll in radians

    pgl_matrix33_t roll_matrix = {
        {cos(roll), sin(roll), 0},
        {-sin(roll), cos(roll), 0},
        {0, 0, 1},
    };
    pgl_matrix33_t pitch_matrix = {
        {1, 0, 0},
        {0, sin(pitch), cos(pitch)},
        {0, cos(pitch), -sin(pitch)},
    };
    // pgl_matrix33_pprint(pgl_matrix33_multiply(roll_matrix, pitch_matrix));
    pgl_matrix33_t yaw_matrix = {
        {cos(yaw), 0, sin(yaw)},
        {0, 1, 0},
        {-sin(yaw), 0, cos(yaw)},
    };
    return pgl_matrix33_multiply(roll_matrix, pgl_matrix33_multiply(pitch_matrix, yaw_matrix));
}

#endif