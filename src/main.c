#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>  // new types: the t in "uint32_t"
#include <stdio.h>

#include "display.h"
#include "vector.h"
#include "triangle.h"
#include "mesh.h"
#include "array.h"

#define BACKGROUND_GRID_INTERVAL 25
#define RED 0xFFFF0000
#define RED_ORANGE 0xFFFF5500
#define YELLOW 0xFFFFFF00
#define GREEN 0xFF00FF00
#define LIGHT_TEAL 0xFF004422
#define TEAL 0xFF00FFCC
#define BLUE 0xFF0000FF
#define BLUE_GREEN 0xFF00FFFF
#define PURPLE 0xFFFF00FF

////////////////////////////////////////////////////////////////////////////////
// Array of triangles to be rendered
////////////////////////////////////////////////////////////////////////////////
triangle_t* triangles_to_render = NULL;

////////////////////////////////////////////////////////////////////////////////
// Global var for exec status and game loop
////////////////////////////////////////////////////////////////////////////////
vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };

float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;

////////////////////////////////////////////////////////////////////////////////
// Initialize vars and objects
////////////////////////////////////////////////////////////////////////////////
void setup(char* object_path) {
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

	// load_cube_mesh_data(); // defined locally
	load_obj_file_data(object_path);
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_e) {
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
	
	// Init array of triangles to render
	triangles_to_render = NULL;

	mesh.rotation.y += .01;
	mesh.rotation.z += .0;
	mesh.rotation.x += .01;

	// loop triangle faces of mesh
	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec3_t transformed_vertices[3];

		// Loop all 3 vertices of current face and apply transformations
		for (int j = 0; j < 3; j++) {
			vec3_t transformed_vertex = face_vertices[j];
			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			// Translate vertex away from camera
			transformed_vertex.z += 5;

			// Store for use outside of loop
			transformed_vertices[j] = transformed_vertex;
		}

		// Cull back-faces
		vec3_t vector_a = transformed_vertices[0];
		vec3_t vector_b = transformed_vertices[1];
		vec3_t vector_c = transformed_vertices[2];

		vec3_t vector_ab = vec3_sub(vector_b, vector_a);
		vec3_t vector_ac = vec3_sub(vector_c, vector_a);
		vec3_normalize(&vector_ab);
		vec3_normalize(&vector_ac);

		// Left-handed coordinate system: take clockwise cross
		// Compute face normal: cross b-a x c-a
		vec3_t normal = vec3_cross(vector_ab, vector_ac);
		vec3_normalize(&normal);

		// Find vector from a point on triangle to camera position
		vec3_t camera_ray = vec3_sub(camera_position, normal);

		// Calculate how aligned camera ray is with face normal: 
		// 	dot product camera ray with face normal
		float dot_normal_camera = vec3_dot(camera_ray, vector_a);

		// Bypass faces that are away from camera
		if (dot_normal_camera < 0) continue;

		// Project each point
		triangle_t projected_triangle;
		for (int j = 0; j < 3; j++) {
			// Project current vertex
			vec2_t projected_point = project(transformed_vertices[j]);

			// Scale and translate projected points to middle of screen (instead of doing it in update)
			projected_point.x += (window_width / 2);
			projected_point.y += (window_height / 2);

			projected_triangle.points[j] = projected_point;
		}

		// Save projected triangle in array of triangles to render
		// triangles_to_render[i] = projected_triangle;
		array_push(triangles_to_render, projected_triangle);
	}
}

void render(void) {
	draw_grid(BACKGROUND_GRID_INTERVAL, LIGHT_TEAL);

	// Loop all projected points and render them
	int num_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_triangles; i++) {
		triangle_t triangle = triangles_to_render[i];

		int x1 = triangle.points[0].x;
		int y1 = triangle.points[0].y;
		int x2 = triangle.points[1].x;
		int y2 = triangle.points[1].y;
		int x3 = triangle.points[2].x;
		int y3 = triangle.points[2].y;

		// printf("x:%d y:%d", x1, y1);
		// printf("before draw rect");
		// fill_rect(x1, y1, 3, 3, RED);
		// fill_rect(x2, y2, 3, 3, RED);
		// fill_rect(x3, y3, 3, 3, RED);

		draw_triangle(x1,y1,x2,y2,x3,y3, RED_ORANGE);
	}
	array_free(triangles_to_render);

	render_color_buffer();
	clear_color_buffer(0xFF111111);
	SDL_RenderPresent(renderer);
}

////////////////////////////////////////////////////////////////////////////////
// Free memory that was dyn alloc
////////////////////////////////////////////////////////////////////////////////
void free_resources() {
   free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(int argc, char* argv[]) {
	char* object_path;
	if (argc > 1) {
		object_path = argv[1];
	} else {
		object_path = "./assets/cube.obj";
	}
	is_running = initialize_window();
	setup(object_path);

	while (is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();

	free_resources();

	return 0;
}