#include "light.h"
#include <stdint.h>  // new types: the t in "uint32_t"
#include "display.h"

light_t light = {
	.direction = {0,0,1}
};

uint32_t light_apply_intensity(color_t triangle_color, float percentage_factor) {
	if (percentage_factor < 0 ) {
		percentage_factor = 0;
	} else if (percentage_factor > 1) {
		percentage_factor = 1;
	}

	uint32_t a = (triangle_color & 0xFF000000);
	uint32_t r = (triangle_color & 0x00FF0000) * percentage_factor;
	uint32_t g = (triangle_color & 0x0000FF00) * percentage_factor;
	uint32_t b = (triangle_color & 0x000000FF) * percentage_factor;

	return a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
}