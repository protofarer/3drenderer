#pragma once

#include "vector.h"
#include "display.h"

// global infinite directional light
typedef struct {
	vec3_t direction;
} light_t;

void init_light(vec3_t direction);
vec3_t get_light_direction(void);
uint32_t light_apply_intensity(uint32_t triangle_color, float intensity_factor);
