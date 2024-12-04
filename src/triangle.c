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

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
	vec2_t ac = vec2_sub(c, a);
	vec2_t ab = vec2_sub(b, a);
	vec2_t pc = vec2_sub(p, c);
	vec2_t pb = vec2_sub(p, b);
	vec2_t ap = vec2_sub(p, a);

	// Area of full parallelogram, cross product
	float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x); // || AC x AB ||
	float area_parallelogram_pbc = (pc.x * pb.y - pc.y * pb.x);
	float area_parallelogram_apc = (ac.x * ap.y - ac.y * ap.x);

	float alpha = area_parallelogram_pbc / area_parallelogram_abc;
	float beta = area_parallelogram_apc / area_parallelogram_abc;
	float gamma = 1.0 - alpha - beta;

	vec3_t weights = { alpha, beta, gamma };

	return weights;
}

// Draw textured pixel at position x and y using interpolation

void draw_texel(
	int x, int y, 
	uint32_t* texture,
	vec4_t point_a, vec4_t point_b, vec4_t point_c, 
	tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
) {
	// float u0, float v0, float u1, float v1, float u2, float v2
	vec2_t p = { x, y };
	vec2_t a = vec2_from_vec4(point_a);
	vec2_t b = vec2_from_vec4(point_b);
	vec2_t c = vec2_from_vec4(point_c);

	vec3_t weights = barycentric_weights(a, b, c, p);

	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	// Variables to store the interpolated values of U, V, 1/w for current pixel
	float interpolated_u;
	float interpolated_v;
	float interpolated_reciprocal_w;

	// Perform interpolate of all U/w and V/w values using barycentric weights
	// and a factor of 1/w
	interpolated_u = ((a_uv.u / point_a.w) * alpha) + ((b_uv.u / point_b.w) * beta) + ((c_uv.u / point_c.w) * gamma);
	interpolated_v = ((a_uv.v / point_a.w) * alpha) + ((b_uv.v / point_b.w) * beta) + ((c_uv.v / point_c.w) * gamma);

	// Also interpolate the value of 1/w for the current pixel
	interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

	// Now we can divide back both interpolated values by 1/w
	interpolated_u /= interpolated_reciprocal_w;
	interpolated_v /= interpolated_reciprocal_w;

	// Map the UV coord to full texture width and height
	int tex_x = abs((int)(interpolated_u * texture_width));
	int tex_y = abs((int)(interpolated_v * texture_height));

	// todo test tex_x and tex_y to prevent buffer overflow
	int i = (texture_width * tex_y) + tex_x;
	if (i < texture_width * texture_height) {
		draw_pixel(x, y, texture[(texture_width * tex_y) + tex_x]);
	}
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
	int x0, int y0, float z0, float w0, float u0, float v0, 
	int x1, int y1, float z1, float w1, float u1, float v1,
	int x2, int y2, float z2, float w2, float u2, float v2, 
	uint32_t* texture
) {
	// order coords by y-coord top->bottom 0->2 :: y0 < y1 < y2
	if (y0 > y1) {
		swap_int(&y0, &y1);
		swap_int(&x0, &x1);
		swap_float(&z0, &z1);
		swap_float(&w0, &w1);
		swap_float(&u0, &u1);
		swap_float(&v0, &v1);
	}
	if (y1 > y2) {
		swap_int(&y1, &y2);
		swap_int(&x1, &x2);
		swap_float(&z1, &z2);
		swap_float(&w1, &w2);
		swap_float(&u1, &u2);
		swap_float(&v1, &v2);
	}
	if (y0 > y1) {
		swap_int(&y0, &y1);
		swap_int(&x0, &x1);
		swap_float(&z0, &z1);
		swap_float(&w0, &w1);
		swap_float(&u0, &u1);
		swap_float(&v0, &v1);
	}

	// Flip the V component to account for inverted UV-coordinates
	v0 = 1.0 - v0;
	v1 = 1.0 - v1;
	v2 = 1.0 - v2;

	// Create vector points and texture coords after vertices sorted
	vec4_t point_a = { x0, y0, z0, w0 };
	vec4_t point_b = { x1, y1, z1, w1 };
	vec4_t point_c = { x2, y2, z2, w2 };
	tex2_t a_uv = { u0, v0 };
	tex2_t b_uv = { u1, v1 };
	tex2_t c_uv = { u2, v2 };

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

			for (int x = x_start; x <= x_end; x++) { // x <= x_end or <, mine differs from lecture "viz textured triangles"
				// draw_pixel(x, y, (x % 4 == 0 && y % 4 == 0) ? BLUE : PURPLE);
				draw_texel(
					x, y, 
					texture,
					point_a, point_b, point_c, 
					a_uv, b_uv, c_uv
				);
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
				// draw_pixel(x, y, (x % 4 == 0 && y % 4 == 0) ? BLUE : PURPLE);
				draw_texel(
					x, y, 
					texture,
					point_a, point_b, point_c, 
					a_uv, b_uv, c_uv
				);
			}
		}
	}
}
