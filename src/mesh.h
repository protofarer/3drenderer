#pragma once

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2) // 6 cube faces, 2 tri per face
extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];

/// ////////////////////////////////////////////////////////////////////////////
// Dynamic size meshes
/// ////////////////////////////////////////////////////////////////////////////
typedef struct {
	vec3_t* vertices; // dynamic array of vertices
	face_t* faces;		// dynamic array of faces
	vec3_t rotation;	// rotation with x, y, and z values
	vec3_t scale;
	vec3_t translation;
} mesh_t;

extern mesh_t mesh;   // the purpose of extern, which is to say "this is declared here, but defined elsewhere."

void load_cube_mesh_data(void);

void load_obj_file_data(char* filepath);


// read vertex lines "v", read in point values into a vertex "index"
// push into dynamic array `vertices`

// read face lines "f", read in first slash values into a face row
// push into dynamic array `faces`