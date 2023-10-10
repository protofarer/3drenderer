#pragma once

#include "vector.h"
#include "display.h"

// global infinite directional light
typedef struct {
	vec3_t direction;
} light_t;

extern light_t light;

uint32_t light_apply_intensity(color_t triangle_color, float intensity_factor);