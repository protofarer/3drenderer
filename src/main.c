#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>  // new types
#include <stdio.h>

#include "display.h"
#include "vector.h"

#define FPS 30
#define FRAME_TARGET_TIME (1000 / FPS)

#define RED 0xFFFF0000
#define RED_ORANGE 0xFFFF5500
#define YELLOW 0xFFFFFF00
#define GREEN 0xFF00FF00
#define LIGHT_TEAL 0xFF007744
#define TEAL 0xFF00FFCC
#define BLUE 0xFF0000FF
#define BLUE_GREEN 0xFF00FFFF
#define PURPLE 0xFFFF00FF

////////////////////////////////////////////////////////////////////////////////
// Declare array of vectors/points
////////////////////////////////////////////////////////////////////////////////

// const int N_POINTS = 9 * 9 * 9;
#define N_POINTS (9 * 9 * 9)
vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];

vec3_t camera_position = { .x = 0, .y = 0, .z = -5 };
vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };

float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;

void setup(void) {
	// Allocate required memory in bytes to hold color buffer
	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);

	// SDL texture used to display color buffer
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);

	int point_count = 0;
	for (float x = -1; x <= 1; x += 0.25) {
		for (float y = -1; y <= 1; y += 0.25) {
			for (float z = -1; z <= 1; z += 0.25) {
					vec3_t new_point = {.x = x, .y = y, .z = z};
					cube_points[point_count++] = new_point;
			}
		}
	}
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				is_running = false;
			}
			break;
	}
}

// Function that receives a 3D vector and returns a projected 2D point
vec2_t project(vec3_t point) {
	vec2_t projected_point = {
		.x = (fov_factor * point.x) / point.z,
		.y = (fov_factor * point.y) / point.z
	};
	return projected_point;
}

void update(void) {
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	previous_frame_time = SDL_GetTicks();

	cube_rotation.y += 0.01;
	cube_rotation.z += 0.01;
	cube_rotation.x += 0.01;

	for (int i = 0; i < N_POINTS; i++) {
		vec3_t point = cube_points[i];

		vec3_t transformed_point = vec3_rotate_y(point, cube_rotation.y);
		transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);
		transformed_point = vec3_rotate_x(transformed_point, cube_rotation.x);

		// move points away from camera
		transformed_point.z -= camera_position.z;
		// Project current poiint
		vec2_t projected_point = project(transformed_point);

		// Save the projected 2d vector in the array of projected points
		projected_points[i] = projected_point;
	}
}

void render(void) {
	draw_grid(25, LIGHT_TEAL);

	// Loop all projected points and render them
	for (int i = 0; i < N_POINTS; i++) {
		vec2_t projected_point = projected_points[i];
		fill_rect(
			projected_point.x + window_width / 2, 
			projected_point.y + window_height / 2, 
			4, 
			4,
			RED_ORANGE
		);
	}

	render_color_buffer();
	clear_color_buffer(0xFF111111);

	SDL_RenderPresent(renderer);
}


int main(void) {
	is_running = initialize_window();

	setup();
	while (is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();
	return 0;
}