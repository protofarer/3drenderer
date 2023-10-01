#pragma once

#include <SDL2/SDL.h>
#include <stdint.h> // new types
#include <stdbool.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern int window_width;
extern int window_height;
extern uint32_t* color_buffer;
extern SDL_Texture* color_buffer_texture;

bool initialize_window(void);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void draw_grid(uint32_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void destroy_window(void);