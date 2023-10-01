#include <stdio.h>
#include <stdint.h> // new types
#include <stdbool.h>
#include <SDL2/SDL.h>

bool is_running = false;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int window_width;
int window_height;

uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;


bool initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0 ) {
		fprintf(stderr, "Error intializing SDL.\n");
		return false;
	}

	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window_width = display_mode.w;
	window_height = display_mode.h;
	
	SDL_Window* window = SDL_CreateWindow(
		NULL, // NULL for no window border (no title)
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width,
		window_height,
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating SDL renderer.\n");
		return false;
	}
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN); // without this, taskbar shows (fake fullscreen)
	
	return true;
}

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

void clear_color_buffer(uint32_t color) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			color_buffer[window_width * y + x] = color;
		}
	}
}

void render_color_buffer(void) {
	SDL_UpdateTexture(
		color_buffer_texture,
		NULL,
		color_buffer,
		(int)window_width * sizeof(uint32_t)
	);
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void draw_grid(uint32_t color) {
	for (int y = 0; y < window_height; y+=1) {
		for (int x = 0; x < window_width; x+=1) {
			if ((y % 10 >= 0 && y % 10 < 4) || (x % 10 >= 0 && x % 10 < 4))
				color_buffer[window_width * y + x] = color;
		}
	}
}

void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
	for (int i = y; i <= y + height; i++) {
		for (int j = x; j <= x + width; j++) {
			if ((i == y || i == y + height) && (j >= x && j <= x + width))
				color_buffer[window_width * i + j] = color;

			if ((j == x || j == x + width) && (i >= y && i <= y + height))
				color_buffer[window_width * i + j] = color;
		}
	}
}

void fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
	for (int i = y; i <= y + height; i++) {
		for (int j = x; j <= x + width; j++) {
				color_buffer[window_width * i + j] = color;
		}
	}
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

void destroy_window(void) {
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();		// destroy init
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