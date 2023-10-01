#include <SDL2/SDL.h>
#include "display.h"
#include <stdio.h>
#include <stdint.h> // new types
#include <stdbool.h>

bool is_running = false;

void setup(void) {
	// Allocate required memory in bytes to hold color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

	// SDL texture used to display color buffer
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);
}

void render(void) {
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_RenderClear(renderer);

	draw_grid((uint32_t)0xFF000099);
	draw_rect(500, 500, 700, 300, 0xFF00FF00);
	fill_rect(1400, 500, 300, 600, 0xFFFF33FF);

	render_color_buffer();
	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				is_running = false;
			break;
	}
}

void update(void) {

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