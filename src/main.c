#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>  // new types: the t in "uint32_t"
#include <stdio.h>

#include "display.h"
#include "vector.h"
#include "triangle.h"
#include "mesh.h"
#include "array.h"
#include "sort.h"
#include "matrix.h"
#include "light.h"

#define M_PI 3.14159265358979323846

#define CAMERA_Z_OFFSET 10

enum cull_method {
	CULL_NONE,
	CULL_BACKFACE
} cull_method;

enum render_method {
	RENDER_WIRE,
	RENDER_WIRE_VERTEX,
	RENDER_FILL_TRIANGLE,
	RENDER_FILL_TRIANGLE_WIRE,
	RENDER_NONE
} render_method;

////////////////////////////////////////////////////////////////////////////////
// Array of triangles to be rendered
////////////////////////////////////////////////////////////////////////////////
triangle_t* triangles_to_render = NULL;

////////////////////////////////////////////////////////////////////////////////
// Global var for exec status and game loop
////////////////////////////////////////////////////////////////////////////////
bool is_running = false;
int previous_frame_time = 0;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };
mat4_t proj_matrix;

////////////////////////////////////////////////////////////////////////////////
// Initialize vars and objects
////////////////////////////////////////////////////////////////////////////////
void setup(char* object_path) {
	is_running = initialize_window();
	cull_method = CULL_BACKFACE;
	render_method = RENDER_FILL_TRIANGLE;

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

	// Initialize perspective projection matrix
	float fov = M_PI / 3; // 60 deg in radians
	float aspect = (float)window_height / (float)window_width;
	float znear = 0.1;
	float zfar = 100.0;
	proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

	// load_obj_file_data(object_path);
	load_obj_file_data("./assets/f22.obj");
	// load_cube_mesh_data(); // defined locally
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
			if (event.key.keysym.sym == SDLK_c) {
				cull_method = true;
			} 
			if (event.key.keysym.sym == SDLK_d) {
				cull_method = false;
			} 
			if (event.key.keysym.sym == SDLK_1) {
				render_method = RENDER_WIRE_VERTEX;
			} 
			if (event.key.keysym.sym == SDLK_2) {
				render_method = RENDER_FILL_TRIANGLE_WIRE;
			} 
			if (event.key.keysym.sym == SDLK_3) {
				render_method = RENDER_WIRE;
			} 
			if (event.key.keysym.sym == SDLK_4) {
				render_method = RENDER_FILL_TRIANGLE;
			} 
			if (event.key.keysym.sym == SDLK_0) {
				render_method = RENDER_NONE;
			}
			break;
	}
}

// Function that receives a 3D vector and returns a projected 2D point
// vec2_t project(vec3_t point) {
// 	vec2_t projected_point = {
// 		.x = (fov_factor * point.x) / point.z,
// 		.y = (fov_factor * point.y) / point.z
// 	};
// 	return projected_point;
// }

// animation params
float angle_total_sweep = 2 * M_PI;
int period = 4000; // 4000ms
int accum_t = 0.0; // accumulated ms up till period
float period_proportion = 0.0; // position relative to period

void update(void) {
	process_input();
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	previous_frame_time = SDL_GetTicks();
	
	// Init array of triangles to render
	triangles_to_render = NULL;

	// todo: angle q that goes from 0 - 2pi over 4 seconds, to be used for setting transformation deltas
	// accum_t = (accum_t + FRAME_TARGET_TIME) % period;
	// period_proportion = (float)accum_t / period;

	// mesh.scale.x += 0.01;
	// mesh.scale.y += 0.001;
	// mesh.scale.z += 0.001;

	// mesh.scale.x = 1 + 0.5 * sin(angle_total_sweep * period_proportion*2);
	// mesh.scale.y = 1 + 0.5 * sin(angle_total_sweep * period_proportion*2);
	// mesh.scale.z = 1 + 0.5 * sin(angle_total_sweep * period_proportion*2);

	// mesh.rotation.y += .05;
	// mesh.rotation.z += .05;
	// mesh.rotation.x += .05;

	// mesh.translation.x = 2 * sin(angle_total_sweep * period_proportion);
	// mesh.translation.y = 2 * cos(angle_total_sweep * period_proportion);

	// Translate vertex away from camera
	mesh.translation.z = CAMERA_Z_OFFSET;

	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);

	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

	// loop triangle faces of mesh
	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec4_t transformed_vertices[3];

		// Loop all 3 vertices of current face and apply transformations
		for (int j = 0; j < 3; j++) {
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

			mat4_t world_matrix = mat4_identity();
			world_matrix = mat4_mult_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mult_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mult_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mult_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mult_mat4(translation_matrix, world_matrix);

			// Multiply by scale matrix
			transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);

			// Multiply by rotation matrices
			transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
			transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
			transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);

			// Multiply by translation matrix
			transformed_vertex = mat4_mul_vec4(translation_matrix, transformed_vertex);

			// Store for use outside of loop
			transformed_vertices[j] = transformed_vertex;
		}

		vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
		vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
		vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

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

		// CULL BACKFACES
		if (cull_method == CULL_BACKFACE) {
			// Bypass/cull faces that are away from camera
			if (dot_normal_camera < 0) continue;
		}

		// PROJECT each point
		vec4_t projected_points[3];
		for (int j = 0; j < 3; j++) {
			// Project current vertex
			// projected_points[j] = project(vec3_from_vec4(transformed_vertices[j]));
			projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

			// Invert y values since window y-coordinate axis is inverted compared to obj file y-axis

			projected_points[j].y *= -1;

			// Scale into view
			projected_points[j].x *= (window_width / 2.0);
			projected_points[j].y *= (window_height / 2.0);

			//Translate projected points to middle of screen
			projected_points[j].x += (window_width / 2.0);
			projected_points[j].y += (window_height / 2.0);

		}

		// Calc average depth of each face based on transformed vertices
		float avg_depth = (transformed_vertices[0].z + 
			transformed_vertices[1].z + transformed_vertices[2].z) / 3;

		//////////////////////////////////////////////////////////////////////////
		// Infinite direction lighting
		//////////////////////////////////////////////////////////////////////////

		// Calc shade intensity based on how aligned is the normal to the inverse of the light
		float light_intensity_factor = -vec3_dot(normal, light.direction);

		color_t triangle_color = mesh_face.color;
		triangle_color = light_apply_intensity(triangle_color, light_intensity_factor);

		triangle_t projected_triangle = {
			.points = {
				{ projected_points[0].x , projected_points[0].y},
				{ projected_points[1].x , projected_points[1].y},
				{ projected_points[2].x , projected_points[2].y}
			},
			.color = triangle_color,
			.avg_depth = avg_depth
		};

		// Save projected triangle in array of triangles to render
		// triangles_to_render[i] = projected_triangle;
		array_push(triangles_to_render, projected_triangle);
	}

	// Painter's Algorithm: Sort triangles to render by avg_depth (hacky render ordering)
	// ? quicksort it
	// bubblesort(triangles_to_render);
	mergesort_triangle(triangles_to_render, 0, array_length(triangles_to_render) - 1);
}


////////////////////////////////////////////////////////////////////////////////
// RENDER
////////////////////////////////////////////////////////////////////////////////
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

		if (
			render_method == RENDER_WIRE_VERTEX || 
			render_method == RENDER_WIRE || 
			render_method == RENDER_FILL_TRIANGLE_WIRE
		) {
			draw_triangle(x1,y1,x2,y2,x3,y3, GREEN);
		} 

		if (
			render_method == RENDER_FILL_TRIANGLE || 
			render_method == RENDER_FILL_TRIANGLE_WIRE
		) {
			draw_filled_triangle(x1,y1,x2,y2,x3,y3, triangle.color);
		}

		if (render_method == RENDER_WIRE_VERTEX) {
			draw_rect(x1 - 3, y1 - 3, 6, 6, PINK);
			draw_rect(x2 - 3, y2 - 3, 6, 6, PINK);
			draw_rect(x3 - 3, y3 - 3, 6, 6, PINK);
		}
	}
	
	// draw_filled_triangle(300+400, 100, 50 + 400, 400, 500 + 400, 700, 0xFFFF00FF);
	// draw_triangle(300, 100, 50, 400, 500, 700, 0xFFFF00FF);
	// flat top
	// draw_filled_triangle(800, 300, 900, 300, 850, 400, 0xFFFF00FF);
	// flat bottom
	// draw_filled_triangle(1000, 500, 1100, 500, 1050, 400, 0xFFFF00FF);

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
	setup(object_path);

	while (is_running) {
		update();
		render();
	}

	destroy_window();

	free_resources();

	return 0;
}