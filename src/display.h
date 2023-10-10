#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>  // new types

typedef uint32_t color_t;

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

#define BACKGROUND_GRID_INTERVAL 25
#define RED 0xFFFF0000
#define RED_ORANGE 0xFFFF5500
#define ORANGE 0xFFFFA500
#define YELLOW 0xFFFFFF00
#define GREEN 0xFF00FF00
#define LIGHT_TEAL 0xFF004422
#define TEAL 0xFF00FFCC
#define BLUE 0xFF0000FF
#define BLUE_GREEN 0xFF00FFFF
#define PURPLE 0xFFFF00FF
#define PINK 0xFFFFC0CB
#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern int window_width;
extern int window_height;
extern uint32_t* color_buffer;
extern SDL_Texture* color_buffer_texture;
extern int fov;

bool initialize_window(void);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void draw_grid(uint32_t interval, uint32_t color);
void draw_gradient_to_black_background(uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void draw_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void destroy_window(void);