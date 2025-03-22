#include "display.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

static uint32_t* color_buffer = NULL;
static float* z_buffer = NULL;

static SDL_Texture* color_buffer_texture = NULL;
static int window_width = 320;
static int window_height = 200;

static int render_method = 0;
static int cull_method = 0;

int get_window_height(void) {
    return window_height;
}

int get_window_width(void) {
    return window_width;
}

void set_render_method(int method) {
    render_method = method;
}

void set_cull_method(int method) {
    cull_method = method;
}

bool is_cull_backface(void) {
    return cull_method == CULL_BACKFACE;
}

bool initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error intializing SDL.\n");
        return false;
    }

    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    // window_width = display_mode.w;
    // window_height = display_mode.h;
    int fullscreen_width = display_mode.w;
    int fullscreen_height = display_mode.h;

    window_width = fullscreen_width / 2;
    window_height = fullscreen_height / 2;

    SDL_Window* window = SDL_CreateWindow(
        NULL,  // NULL for no window border (no title)
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        fullscreen_width,
        fullscreen_height,
        SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);  // without this, taskbar shows (fake fullscreen)

	// Allocate required memory in bytes to hold color buffer
	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
	z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

	// SDL texture used to display color buffer
	color_buffer_texture = SDL_CreateTexture(
		renderer, 
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);

    return true;
}

void render_color_buffer(void) {
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int)window_width * sizeof(uint32_t));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void clear_color_buffer(uint32_t color) {
    // for (int y = 0; y < window_height; y++) {
    //     for (int x = 0; x < window_width; x++) {
    //         color_buffer[window_width * y + x] = color;
    //     }
    // }
    for (int i = 0; i < window_height * window_width; i++) {
        color_buffer[i] = color;
    }
}

void clear_z_buffer() {
    // for (int y = 0; y < window_height; y++) {
    //     for (int x = 0; x < window_width; x++) {
    //         z_buffer[window_width * y + x] = 1.0;
    //     }
    // }
    for (int i = 0; i < window_height * window_width; i++) {
        z_buffer[i] = 1.0;
    }
}

void draw_grid(uint32_t interval, uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            if (y % interval == 0 || x % interval == 0)
                color_buffer[window_width * y + x] = color;
        }
    }
}

// TODO this is broken, fix ratio calculation
void draw_gradient_to_black_background(uint32_t color) {
    uint8_t original_red = (color & 0x00FF0000) >> 16;
    uint8_t original_green = (color & 0x0000FF00) >> 8;
    uint8_t original_blue = (color & 0x000000FF);
    // get total number of rows
    // calculate a smooth delta for each input color R, G, B to 00 for each
    // that is: over window_height, decrease RR, GG, BB to zero over appropriate
    // interval, probably using a conditional modulus test where the modulus
    // factor is the total number of smallest decrements available to reduce
    // exactly to black at the bottom of screen
    
    for (int i = 0; i <= window_height; i++) {
        float ratio = 1.0f - ((float)(i) / (float)window_height);

        uint8_t red = (uint8_t)(original_red * ratio);
        uint8_t green = (uint8_t)(original_green * ratio);
        uint8_t blue = (uint8_t)(original_blue * ratio);

        uint32_t gradient_color = 0xFF000000 | (red << 16) | (green << 8) | blue;

        for (int j = 0; j <= window_width; j++) {
            color_buffer[window_width * i + j] = gradient_color;
        }
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
        return;
    }
    color_buffer[window_width * y + x] = color;
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

void draw_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (int i = y; i <= y + height; i++) {
        for (int j = x; j <= x + width; j++) {
            draw_pixel(j, i, color);
        }
    }
}

// naive DDA line algorithm
void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int x_len = (x1 - x0);
    int y_len = (y1 - y0);

    int longer_side_length = abs(x_len) >= abs(y_len) ? abs(x_len) : abs(y_len);

    float dx = x_len / (float)longer_side_length;
    float dy = y_len / (float)longer_side_length;

    float x = x0;
    float y = y0;
    for (int i = 0; i <= longer_side_length; i++) {
        draw_pixel(round(x), round(y), color);
        x += dx;
        y += dy;
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);

}

void destroy_window(void) {
	free(color_buffer);
	free(z_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();  // destroy init
}

bool should_render_textured_triangles(void) {
	return (
        render_method == RENDER_TEXTURED || 
        render_method == RENDER_TEXTURED_WIRE
   );
}

bool should_render_wireframe(void) {
    return (
        render_method == RENDER_WIRE || 
        render_method == RENDER_WIRE_VERTEX || 
        render_method == RENDER_FILL_TRIANGLE_WIRE ||
        render_method == RENDER_TEXTURED_WIRE
   );
}

bool should_render_filled_triangles(void) {
    return (
        render_method == RENDER_FILL_TRIANGLE || 
        render_method == RENDER_FILL_TRIANGLE_WIRE
   );
}

bool should_render_wire_vertex(void) {
    return render_method == RENDER_WIRE_VERTEX;
}

float get_zbuffer_at(int x, int y) {
    // if garbage inputs, return starting value (1.0)
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
        return 1.0;
    }
    return z_buffer[(window_width * y) + x];
}

void update_zbuffer_at(int x, int y, float value) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
        return;
    }
    z_buffer[(window_width * y) + x] = value;
}
