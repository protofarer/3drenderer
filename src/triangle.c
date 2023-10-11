#include <stdint.h>
#include "triangle.h"
#include "display.h"
#include "swap.h"

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom
///////////////////////////////////////////////////////////////////////////////
//
//        (x0,y0)
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void fill_flat_bottom_triangle(
	int x0, int y0, 
	int x1, int y1, 
	int x2, int y2, 
	color_t color
) {
	// float inv_slope_0_1 = (float)(x0 - x1) / (y0 - y1);
	// float inv_slope_0_m = (float)(xm - x0) / (ym - y0);
	float inv_slope_0_1 = (float)(x1 - x0) / (y1 - y0);
	float inv_slope_0_m = (float)(x2 - x0) / (y2 - y0);

	float x_start = x0;
	float x_end = x0;
	for (int y = y0; y <= y2; y++) {
		draw_line(x_start, y, x_end, y, color);
		x_start += inv_slope_0_1;
		x_end += inv_slope_0_m; 
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top
///////////////////////////////////////////////////////////////////////////////
//
//  (x0,y0)------(x1,y1)
//      \         /
//       \       /
//        \     /
//         \   /
//          \ /
//        (x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void fill_flat_top_triangle(
	int x0, int y0, 
	int x1, int y1, 
	int x2, int y2, 
	color_t color
) {
	float inv_slope_1_0 = (float)(x2 - x0) / (y2 - y0);
	float inv_slope_0_m = (float)(x2 - x1) / (y2 - y1);

	float x_start = x2;
	float x_end = x2;
	for (int y = y2; y >= y0; y--) { // ? unsure if equal to needed since the top/bottom faces share a line that fill_flat already covers
		draw_line(x_start, y, x_end, y, color);
		x_start -= inv_slope_1_0;
		x_end -= inv_slope_0_m; 
	}
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, color_t color) {
	// y0 > y1 > y2
	if (y0 > y1) {
		swap_int(&y0, &y1);
		swap_int(&x0, &x1);
	}
	if (y1 > y2) {
		swap_int(&y1, &y2);
		swap_int(&x1, &x2);
	}
	if (y0 > y1) {
		swap_int(&y0, &y1);
		swap_int(&x0, &x1);
	}

	// if ((y2 - y0) == 0) return;	// ? does culling take care of this?

	if (y1 == y2) {
		fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
	} else if (y0 == y1) {
		fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
	} else {
		// Calculate new vertex (Mx, My) using similar triangles
		// int xm = (((float)(y1 - y0) * (x2 - x0)) / (float)(y2 - y0)) + x0;
		int xm = (((y1 - y0) * (x2 - x0)) / (y2 - y0)) + x0;
		int ym = y1;

		fill_flat_bottom_triangle(x0, y0, x1, y1, xm, ym, color);
		fill_flat_top_triangle(x1, y1, xm, ym, x2, y2, color);
	}
}

void texture_flat_bottom_triangle(
	int x0, int y0, float u0, float v0, 
	int x1, int y1, float u1, float v1,
	int x2, int y2, float u2, float v2, 
	uint32_t* texture
) {
	// for every pixel on the face, draw a texel

}
///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method
// We split the original triangle in two, half flat-bottom and half flat-top
///////////////////////////////////////////////////////////////////////////////
//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)------(Mx,My)
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void draw_textured_triangle(
	int x0, int y0, float u0, float v0, 
	int x1, int y1, float u1, float v1,
	int x2, int y2, float u2, float v2, 
	uint32_t* texture
) {
	// order coords by y-coord top->bottom 0->2 :: y0 < y1 < y2
	if (y0 > y1) {
		swap_int(&y0, &y1);
		swap_int(&x0, &x1);
		swap_float(&u0, &u1);
		swap_float(&v0, &v1);
	}
	if (y1 > y2) {
		swap_int(&y1, &y2);
		swap_int(&x1, &x2);
		swap_float(&u1, &u2);
		swap_float(&v1, &v2);
	}
	if (y0 > y1) {
		swap_int(&y0, &y1);
		swap_int(&x0, &x1);
		swap_float(&u0, &u1);
		swap_float(&v0, &v1);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Render upper half of triangle (flat-bottom)
	///////////////////////////////////////////////////////////////////////////////
	float inv_slope_1;
	float inv_slope_2;
	if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
	if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
	if (y1 - y0 != 0) {
		for (int y = y0; y <= y1; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			if (x_end < x_start) swap_int(&x_end, &x_start);

			for (int x = x_start; x <= x_end; x++) {
				draw_pixel(x, y, (x % 4 == 0 && y % 4 == 0) ? BLUE : PURPLE);
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	// Render lower half of triangle (flat-top)
	///////////////////////////////////////////////////////////////////////////////
	if (y2 - y1 != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
	if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
	if (y2 - y1 != 0) {
		for (int y = y1; y <= y2; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			if (x_end < x_start) swap_int(&x_end, &x_start);

			for (int x = x_start; x <= x_end; x++) {
				draw_pixel(x, y, (x % 4 == 0 && y % 4 == 0) ? BLUE : PURPLE);
			}
		}
	}
}