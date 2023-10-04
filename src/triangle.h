#pragma once

#include <stdint.h>  // new types: the t in "uint32_t"
#include "vector.h"

typedef struct {
	int a;
	int b;
	int c;
} face_t;

typedef struct {
	vec2_t points[3];
} triangle_t;


void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int xm, int ym, uint32_t color);
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int xm, int ym, uint32_t color);
