#include <stdint.h>
#include "triangle.h"
#include "display.h"


void swap(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void fill_flat_bottom_triangle(
	int x0, int y0, 
	int x1, int y1, 
	int xm, int ym, 
	color_t color
) {
	float inv_slope_0_1 = (float)(x0 - x1) / (y0 - y1);
	float inv_slope_0_m = (float)(xm - x0) / (ym - y0);

	float x_start = x0;
	float x_end = x0;
	for (int y = y0; y <= y1; y++) {
		draw_line(x_start, y, x_end, y, color);
		x_start += inv_slope_0_1;
		x_end += inv_slope_0_m; 
	}
}

void fill_flat_top_triangle(
	int x1, int y1, 
	int x2, int y2, 
	int xm, int ym, 
	color_t color
) {
	float inv_slope_1_0 = (float)(x2 - x1) / (y2 - y1);
	float inv_slope_0_m = (float)(x2 - xm) / (y2 - ym);

	float x_start = x2;
	float x_end = x2;
	for (int y = y2; y >= y1; y--) { // ? unsure if equal to needed since the top/bottom faces share a line that fill_flat already covers
		draw_line(x_start, y, x_end, y, color);
		x_start -= inv_slope_1_0;
		x_end -= inv_slope_0_m; 
	}
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, color_t color) {
	// y0 > y1 > y2
	if (y0 > y1) {
		swap(&y0, &y1);
		swap(&x0, &x1);
	}
	if (y1 > y2) {
		swap(&y1, &y2);
		swap(&x1, &x2);
	}
	if (y0 > y1) {
		swap(&y0, &y1);
		swap(&x0, &x1);
	}

	// if ((y2 - y0) == 0) return;	// ? does culling take care of this?

	if (y0 == y1) {
		fill_flat_top_triangle(x1, y1, x2, y2, x0, y0, color);
	} else if (y1 == y2) {
		fill_flat_bottom_triangle(x1, y1, x2, y2, x0, y0, color);
	} else {
		// Calculate new vertex (Mx, My) using similar triangles
		// int xm = (((float)(y1 - y0) * (x2 - x0)) / (float)(y2 - y0)) + x0;
		int xm = (((y1 - y0) * (x2 - x0)) / (y2 - y0)) + x0;
		int ym = y1;

		fill_flat_bottom_triangle(x0, y0, x1, y1, xm, ym, color);
		fill_flat_top_triangle(x1, y1, x2, y2, xm, ym, color);
	}
}
