#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>  // new types: the t in "uint32_t"

#include "clipping.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "triangle.h"
#include "texture.h"
#include "camera.h"

#define M_PI 3.14159265358979323846

#define CAMERA_Z_OFFSET 5

// tmp: workaround to address gcc error when this is located in display.h
enum cull_method {
	CULL_NONE,
	CULL_BACKFACE
} cull_method;

// tmp: workaround to address gcc error when this is located in display.h
enum render_method {
	RENDER_WIRE,
	RENDER_WIRE_VERTEX,
	RENDER_FILL_TRIANGLE,
	RENDER_FILL_TRIANGLE_WIRE,
	RENDER_TEXTURED,
	RENDER_TEXTURED_WIRE,
	RENDER_NONE
} render_method;

////////////////////////////////////////////////////////////////////////////////
// Array of triangles to be rendered each frame
////////////////////////////////////////////////////////////////////////////////
#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

////////////////////////////////////////////////////////////////////////////////
// Global var for exec status and game loop
////////////////////////////////////////////////////////////////////////////////
bool is_running = false;
float delta_time = 0.0;
int previous_frame_time = 0;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };

///////////////////////////////////////////////////////////////////////////////
// Declaration of our global transformation matrices
///////////////////////////////////////////////////////////////////////////////
mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

////////////////////////////////////////////////////////////////////////////////
// Initialize vars and objects
////////////////////////////////////////////////////////////////////////////////
void setup(char* object_path) {
	is_running = initialize_window();
	cull_method = CULL_BACKFACE;
	render_method = RENDER_TEXTURED;

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

	// Initialize perspective projection matrix
	float fov = M_PI / 3; // 60 deg in radians
	float aspect = (float)window_height / (float)window_width;
	float z_near = 0.1;
	float z_far = 100.0;
	proj_matrix = mat4_make_perspective(fov, aspect, z_near, z_far);

	// Initialize frustrum planes with point and normal each
	init_frustrum_planes(fov, z_near, z_far);


	// Manually load hardcoded texture data from the static array
	// mesh_texture = (uint32_t*)REDBRICK_TEXTURE;
	// texture_width = 64;
	// texture_height = 64;

	// Loads the vertex and face values for the mesh data structure
	// load_obj_file_data(object_path);
	// load_obj_file_data("./assets/f117.obj");
	load_obj_file_data("./assets/cube.obj");
	// load_cube_mesh_data(); // defined locally

	// Load the texture information from an external PNG file
	// load_png_texture_data("./assets/f117.png");
	load_png_texture_data("./assets/cube.png");
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
			if (event.key.keysym.sym == SDLK_c)
				cull_method = CULL_BACKFACE; 
			if (event.key.keysym.sym == SDLK_x)
				cull_method = CULL_NONE; 
			if (event.key.keysym.sym == SDLK_1)
				render_method = RENDER_WIRE_VERTEX; 
			if (event.key.keysym.sym == SDLK_2)
				render_method = RENDER_FILL_TRIANGLE_WIRE; 
			if (event.key.keysym.sym == SDLK_3)
				render_method = RENDER_WIRE; 
			if (event.key.keysym.sym == SDLK_4)
				render_method = RENDER_FILL_TRIANGLE; 
			if (event.key.keysym.sym == SDLK_5)
				render_method = RENDER_TEXTURED; 
			if (event.key.keysym.sym == SDLK_6)
				render_method = RENDER_TEXTURED_WIRE; 
			if (event.key.keysym.sym == SDLK_0)
				render_method = RENDER_NONE;
			if (event.key.keysym.sym == SDLK_r)
				camera.position.y += 3.0 * delta_time;
			if (event.key.keysym.sym == SDLK_f)
				camera.position.y -= 3.0 * delta_time;
			if (event.key.keysym.sym == SDLK_a)
				camera.yaw -= 1.0 * delta_time;
			if (event.key.keysym.sym == SDLK_d)
				camera.yaw += 1.0 * delta_time;
			if (event.key.keysym.sym == SDLK_w) {
				camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
				camera.position = vec3_add(camera.position, camera.forward_velocity);
			}
			if (event.key.keysym.sym == SDLK_s) {
				camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
				camera.position = vec3_sub(camera.position, camera.forward_velocity);
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

	delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

	previous_frame_time = SDL_GetTicks();

	// Initialize the counter of triangles to render for current rame
	num_triangles_to_render = 0;

	// todo: angle q that goes from 0 - 2pi over 4 seconds, to be used for setting transformation deltas
	// accum_t = (accum_t + FRAME_TARGET_TIME) % period;
	// period_proportion = (float)accum_t / period;

	// mesh.scale.x += 0.01;
	// mesh.scale.y += 0.001;
	// mesh.scale.z += 0.001;

	// mesh.scale.x = 1 + 0.5 * sin(angle_total_sweep * period_proportion*2);
	// mesh.scale.y = 1 + 0.5 * sin(angle_total_sweep * period_proportion*2);
	// mesh.scale.z = 1 + 0.5 * sin(angle_total_sweep * period_proportion*2);

	// mesh.rotation.x += .025;
	// mesh.rotation.y += 1.0 * delta_time;
	/* mesh.rotation.z += .05; */

	// mesh.translation.x = 2 * sin(angle_total_sweep * period_proportion);
	// mesh.translation.y = 2 * cos(angle_total_sweep * period_proportion);

	// Translate vertex away from camera
	mesh.translation.z = CAMERA_Z_OFFSET;

	// Initialize target looking at pos z-axis
	vec3_t target = { 0, 0, 1 };
	mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
	camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));

	// Offset cam pos in dir where cam is pointing at
	target = vec3_add(camera.position, camera.direction);
	vec3_t up_direction = { 0, 1, 0 };

	// Create the view matrix
	view_matrix = mat4_look_at(camera.position, target, up_direction);

	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);


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

			// Create world matrix w/ scale, rotate, translate matrices
			mat4_t world_matrix = mat4_identity();

			// Order matters: scale, rotate, translate [T]*[R]*[S]*v
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			///////////// OLD
			// Multiply by scale matrix
			// transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
			//
			// // Multiply by rotation matrices
			// transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
			// transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
			// transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);
			//
			// // Multiply by translation matrix
			// transformed_vertex = mat4_mul_vec4(translation_matrix, transformed_vertex);
			///////////////////////////////

			//////////// NEW BUT BUG (instructor must have changed previous lessons' code that I'm unaware of)
			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
			transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

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

		// Create a polygon from original transformed triangle to be clipped
		polygon_t polygon = polygon_from_triangle(
			vec3_from_vec4(transformed_vertices[0]), 
			vec3_from_vec4(transformed_vertices[1]), 
			vec3_from_vec4(transformed_vertices[2])
		);

		// Clip poly and return new poly with potential new vertices
		clip_polygon(&polygon);

		// Break polygon apart back into triangles
		triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
		int num_triangles_after_clipping = 0;

		triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

		// Loops all assembled triangles after clipping
		for (int t = 0; t < num_triangles_after_clipping; t++) {
			triangle_t triangle_after_clipping = triangles_after_clipping[t];

			// PROJECT each point
			vec4_t projected_points[3];

			// Loop all 3 vertices to perform projection and conversion to screen space
			for (int j = 0; j < 3; j++) {
				// Project current vertex
				// projected_points[j] = project(vec3_from_vec4(transformed_vertices[j]));
				projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

				// Invert y values since window y-coordinate axis is inverted compared to obj file y-axis

				projected_points[j].y *= -1;

				// Scale into view
				projected_points[j].x *= (window_width / 2.0);
				projected_points[j].y *= (window_height / 2.0);

				//Translate projected points to middle of screen
				projected_points[j].x += (window_width / 2.0);
				projected_points[j].y += (window_height / 2.0);

			}

			//////////////////////////////////////////////////////////////////////////
			// Infinite direction lighting
			//////////////////////////////////////////////////////////////////////////

			// Calc shade intensity based on how aligned is the normal to the inverse of the light
			float light_intensity_factor = -vec3_dot(normal, light.direction);

			color_t triangle_color = mesh_face.color;
			triangle_color = light_apply_intensity(triangle_color, light_intensity_factor);

			triangle_t triangle_to_render = {
				.points = {
					{ projected_points[0].x , projected_points[0].y, projected_points[0].z, projected_points[0].w},
					{ projected_points[1].x , projected_points[1].y, projected_points[1].z, projected_points[1].w},
					{ projected_points[2].x , projected_points[2].y, projected_points[2].z, projected_points[2].w},
				},
				.texcoords = {
					{ mesh_face.a_uv.u, mesh_face.a_uv.v }, 
					{ mesh_face.b_uv.u, mesh_face.b_uv.v }, 
					{ mesh_face.c_uv.u, mesh_face.c_uv.v },
				},
				.color = triangle_color,
			};

			// Save projected triangle in array of triangles to render
			if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
				triangles_to_render[num_triangles_to_render] = triangle_to_render;
				num_triangles_to_render++;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// RENDER
////////////////////////////////////////////////////////////////////////////////
void render(void) {
	SDL_RenderClear(renderer);
	draw_grid(BACKGROUND_GRID_INTERVAL, LIGHT_TEAL);

	// Loop all projected points and render them
	for (int i = 0; i < num_triangles_to_render; i++) {
		triangle_t triangle = triangles_to_render[i];
		
		int x[3], y[3];
		float z[3], w[3];
		tex2_t uv[3];

		for (int j = 0; j < 3; j++) {
			x[j] = triangle.points[j].x;
			y[j] = triangle.points[j].y;
			z[j] = triangle.points[j].z;
			w[j] = triangle.points[j].w;
			uv[j] = triangle.texcoords[j];
		}

		// Draw textured triangle
		if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE) {
			// don't actually need to pass z's
			draw_textured_triangle(
				x[0], y[0], z[0], w[0], uv[0].u, uv[0].v,
				x[1], y[1], z[1], w[1], uv[1].u, uv[1].v,
				x[2], y[2], z[2], w[2], uv[2].u, uv[2].v,
				mesh_texture
			);
		}

		// Draw wires
		if (
			render_method == RENDER_WIRE_VERTEX || 
			render_method == RENDER_WIRE || 
			render_method == RENDER_FILL_TRIANGLE_WIRE ||
			render_method == RENDER_TEXTURED_WIRE
		) {
			draw_triangle(x[0], y[0], x[1], y[1], x[2], y[2], GREEN);
		} 

		if (
			render_method == RENDER_FILL_TRIANGLE || 
			render_method == RENDER_FILL_TRIANGLE_WIRE
		) {
			// don't actually need to pass z's here either (see draw_textured_triangle)
			draw_filled_triangle(
				x[0],y[0],w[0],
				x[1],y[1],w[1],
				x[2],y[2],w[2], 
				triangle.color
			);
		}

		if (render_method == RENDER_WIRE_VERTEX) {
			draw_rect(x[0] - 3, y[0] - 3, 6, 6, PINK);
			draw_rect(x[1] - 3, y[1] - 3, 6, 6, PINK);
			draw_rect(x[2] - 3, y[2] - 3, 6, 6, PINK);
		}
	}
	
	render_color_buffer();
	clear_color_buffer(0xFF111111);
	clear_z_buffer();
	SDL_RenderPresent(renderer);
}

////////////////////////////////////////////////////////////////////////////////
// Free memory that was dyn alloc
////////////////////////////////////////////////////////////////////////////////
void free_resources() {
	free(color_buffer);
	free(z_buffer);
	upng_free(png_texture);
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
