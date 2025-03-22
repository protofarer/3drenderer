#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>  // new types

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

// tmp: workaround to address gcc error when this is located in display.h
enum cull_method {
	CULL_NONE,
	CULL_BACKFACE
};

// tmp: workaround to address gcc error when this is located in display.h
enum render_method {
	RENDER_WIRE,
	RENDER_WIRE_VERTEX,
	RENDER_FILL_TRIANGLE,
	RENDER_FILL_TRIANGLE_WIRE,
	RENDER_TEXTURED,
	RENDER_TEXTURED_WIRE,
	RENDER_NONE
};


// extern SDL_Window* window;
// extern SDL_Renderer* renderer;
// extern int window_width;
// extern int window_height;
// extern uint32_t* color_buffer;
// extern float* z_buffer;
// extern SDL_Texture* color_buffer_texture;
// extern int fov;

bool initialize_window(void);
int get_window_height(void);
int get_window_width(void);
void destroy_window(void);

void set_render_method(int method);
void set_cull_method(int method);
bool is_cull_backface(void);

bool should_render_textured_triangles(void);
bool should_render_wireframe(void);
bool should_render_filled_triangles(void);
bool should_render_wire_vertex(void);

void draw_grid(uint32_t interval, uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void draw_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void draw_gradient_to_black_background(uint32_t color);

void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void clear_z_buffer(void);
float get_zbuffer_at(int x, int y);
void update_zbuffer_at(int x, int y, float value);
