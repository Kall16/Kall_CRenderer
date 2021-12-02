#ifndef MESH_H
#define MESH_H

#include "Vector.h"
#include "Triangle.h"
#include "Array.h"
#include "upng.h"


/// <summary>
/// ///// define a struct for dynamic size meshes, with array of vertices and faces
/// </summary>

typedef struct
{
	vec3_t* vertices;	// mesh dynamic array of vertices
	face_t* faces;		// mesh dynamic array of faces
	upng_t* texture;	// mesh PNG texture pointer
	vec3_t rotation;	// mesh rotation with x, y, z values
	vec3_t scale;		// mesh scale with x, y, z values
	vec3_t translation;	// mesh translation with x, y, z values
} mesh_t;

void load_mesh(char* obj_filename, char* png_filename, vec3_t scale, vec3_t translation, vec3_t rotation);
void load_mesh_obj_data(mesh_t* mesh, char* filename);
void load_mesh_png_data(mesh_t* mesh, char* filename);

int get_num_meshes(void);
mesh_t* get_mesh(int index);

void free_meshes(void);


#endif
