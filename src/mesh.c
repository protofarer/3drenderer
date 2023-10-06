#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mesh.h"
#include "array.h"
#include "display.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = { 0, 0, 0 },
    .scale = { 1.0, 1.0, 1.0 },
    .translation = { 0, 0, 0 }
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
	{ .x = -1, .y = -1, .z = -1}, // 1
	{ .x = -1, .y =  1, .z = -1},
	{ .x =  1, .y =  1, .z = -1},
	{ .x =  1, .y = -1, .z = -1}, // ...
	{ .x =  1, .y =  1, .z =  1},
	{ .x =  1, .y = -1, .z =  1},
	{ .x = -1, .y =  1, .z =  1},
	{ .x = -1, .y = -1, .z =  1}, // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3, .color = PURPLE },
    { .a = 1, .b = 3, .c = 4, .color = PURPLE },
    // right
    { .a = 4, .b = 3, .c = 5, .color = GREEN },
    { .a = 4, .b = 5, .c = 6, .color = GREEN },
    // back
    { .a = 6, .b = 5, .c = 7, .color = BLUE },
    { .a = 6, .b = 7, .c = 8, .color = BLUE },
    // left
    { .a = 8, .b = 7, .c = 2, .color = YELLOW },
    { .a = 8, .b = 2, .c = 1, .color = YELLOW },
    // top
    { .a = 2, .b = 7, .c = 5, .color = RED },
    { .a = 2, .b = 5, .c = 3, .color = RED },
    // bottom
    { .a = 6, .b = 8, .c = 1, .color = ORANGE },
    { .a = 6, .b = 1, .c = 4, .color = ORANGE }
};

void load_cube_mesh_data(void) {
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        vec3_t cube_vertex = cube_vertices[i];
        array_push(mesh.vertices, cube_vertex);
    }
    for (int i = 0; i < N_CUBE_FACES; i++) {
        face_t cube_face = cube_faces[i];
        array_push(mesh.faces, cube_face);
    }
}

// read vertex lines "v", read face lines "f"
// read in point values into a vertex "index", push into dynamic array `vertices`
// read in first slash values into a face row, push into dynamic array `faces`
void load_obj_file_data(char* filepath) {
    // read file from disk
    FILE* file = fopen(filepath, "r");
    if (file == NULL) return;

    // initial allocation
    size_t buffer_size = 64;    // starting value based on cube file, can increase buffer as longer lines are encountered
    char* line = malloc(buffer_size);
    if (line == NULL) {     // catch malloc fail, unlikely
        fclose(file);
        printf("FAIL initial malloc");
        return;
    }

    // open text file
        int line_n = 0;
    while (!feof(file)) {
        if (fgets(line, buffer_size, file)) {
            // read line by line and parse

            // double buffer if buffer limit hit
            if (line[buffer_size - 2] != '\0' && line[buffer_size - 2] != '\n') {
                buffer_size *= 2;
                char* new_line = realloc(line, buffer_size);
                if (new_line == NULL) {
                    free(line);
                    fclose(file);
                    printf("FAIL realloc increase buffer size");
                    return;
                }
                line = new_line;
            }

            // PARSE

            // get first token: returns ptr to 1st token string in line
            char* token = strtok(line, " ");
            if (token == NULL) continue;

            // printf("----- line[%d] 1st token: \"%s\" -----\n", line_n, token);

            // parse vertex line
            if (strcmp(token,"v") == 0) {
                float vertices[3]; // stack allocate and initialize
                int i = 0;
                while (token != NULL) {
                    token = strtok(NULL, " "); // read until space
                    if (token == NULL) break;
                    // printf("%s\n", token);

                    float vertex = atof(token);
                    vertices[i] = vertex;
                    i++;
                }
                vec3_t vec3_vertices = {
                    .x = vertices[0],
                    .y = vertices[1],
                    .z = vertices[2],
                };
                array_push(mesh.vertices, vec3_vertices);
                continue;
            }

            // parse face line
            if (strcmp(token, "f") == 0) {
                int face[3];
                int vt[3];
                int vn[3];
                int i = 0;
                while (token != NULL) {
                    token = strtok(NULL, " "); // read until space
                    if (token == NULL) break;

                    // strtok is not re-entrant, do manual parsing via sscanf, it is also extensible to other values on the "f" line
                    sscanf(token, "%d/%d/%d", &face[i], &vt[i], &vn[i]);
                    // printf("face[%d]: %d\n", i, face[i]);
                    i++;
                }
                face_t mesh_face = {
                    .a = face[0],
                    .b = face[1],
                    .c = face[2]
                };
                array_push(mesh.faces, mesh_face);
                continue;
            }
        }

        line_n++;
    }

    // push appropriate line data to appropriate array (vertices or faces)

    // free and close
    free(line);
    fclose(file);
}

// void load_obj_file_data(char* filename) {
//     FILE* file;
//     file = fopen(filename, "r");
//     char line[1024];

//     while (fgets(line, 1024, file)) {
//         // Vertex information
//         if (strncmp(line, "v ", 2) == 0) {
//             vec3_t vertex;
//             sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
//             array_push(mesh.vertices, vertex);
//         }
//         // Face information
//         if (strncmp(line, "f ", 2) == 0) {
//             int vertex_indices[3];
//             int texture_indices[3];
//             int normal_indices[3];
//             sscanf(
//                 line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
//                 &vertex_indices[0], &texture_indices[0], &normal_indices[0], 
//                 &vertex_indices[1], &texture_indices[1], &normal_indices[1], 
//                 &vertex_indices[2], &texture_indices[2], &normal_indices[2]
//             ); 
//             face_t face = {
//                 .a = vertex_indices[0],
//                 .b = vertex_indices[1],
//                 .c = vertex_indices[2]
//             };
//             array_push(mesh.faces, face);
//         }
//     }
// }