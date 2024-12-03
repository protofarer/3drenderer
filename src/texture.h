#pragma once

#include <stdint.h>
#include "upng.h"

typedef struct {
	float u, v;
} tex2_t;

extern int texture_width;
extern int texture_height;

extern upng_t* png_texture;
extern uint32_t* mesh_texture;

extern const uint8_t REDBRICK_TEXTURE[];

void load_png_texture_data(char* filename);
