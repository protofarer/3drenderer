#pragma once

#include "triangle.h"
#include "vector.h"

#define MAX_NUM_POLY_VERTICES 10
#define MAX_NUM_POLY_TRIANGLES 10

enum {
    LEFT_FRUSTRUM_PLANE,
    RIGHT_FRUSTRUM_PLANE,
    TOP_FRUSTRUM_PLANE,
    BOTTOM_FRUSTRUM_PLANE,
    NEAR_FRUSTRUM_PLANE,
    FAR_FRUSTRUM_PLANE 
};

typedef struct {
    vec3_t point;
    vec3_t normal;
} plane_t;

typedef struct {
    vec3_t vertices[MAX_NUM_POLY_VERTICES];
    int num_vertices;
} polygon_t;

void init_frustrum_planes(float fov_x, float fov_y, float z_near, float z_far);
polygon_t polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2);
void clip_polygon(polygon_t* polygon);
void clip_polygon_against_plane(polygon_t* polygon, int plane);
void triangles_from_polygon(polygon_t* polygon, triangle_t triangles[], int* num_triangles);
