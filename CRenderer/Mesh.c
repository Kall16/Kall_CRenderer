#include "Mesh.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MAX_NUM_MESHES 10
static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

void load_mesh(char* obj_filename, char* png_filename, vec3_t scale, vec3_t translation, vec3_t rotation)
{
	load_mesh_obj_data(&meshes[mesh_count], obj_filename);
	load_mesh_png_data(&meshes[mesh_count], png_filename);
	meshes[mesh_count].scale = scale;
	meshes[mesh_count].translation = translation;
	meshes[mesh_count].rotation = rotation;

	mesh_count++;
}

void load_mesh_obj_data(mesh_t* mesh, char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "r");
	//file = fopen(filename, "r");

	char line[1024];

	tex2_t* texcoords = NULL;
	vec3_t* normals = NULL;
	//vec3_t* vert = NULL;

	while (fgets(line, 1024, file))
	{
		// Vertex Information
		if (strncmp(line, "v ", 2) == 0)
		{
			vec3_t vertex;
			sscanf_s(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			array_push(mesh->vertices, vertex);
			//array_push(vert, vertex);
		}

		// Texture coordinate information
		if (strncmp(line, "vt ", 3) == 0)
		{
			tex2_t texcoord;
			sscanf_s(line, "vt %f %f", &texcoord.u, &texcoord.v);
			array_push(texcoords, texcoord);

		}

		// Normal information
		if (strncmp(line, "vn ", 3) == 0)
		{
			vec3_t normal;
			sscanf_s(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
			normal = vec3_normalize(normal);

			array_push(normals, normal);
		}		

		if (strncmp(line, "f ", 2) == 0)
		{
			int vertex_indices[3];
			int texture_indices[3];
			sscanf_s(line, "f %d/%d %d/%d %d/%d",
				&vertex_indices[0], &texture_indices[0],
				&vertex_indices[1], &texture_indices[1],
				&vertex_indices[2], &texture_indices[2]
			);

			face_t face =
			{
				.a = vertex_indices[0] - 1,
				.b = vertex_indices[1] - 1,
				.c = vertex_indices[2] - 1,
				.a_uv = texcoords[texture_indices[0] - 1],
				.b_uv = texcoords[texture_indices[1] - 1],
				.c_uv = texcoords[texture_indices[2] - 1],
				.color = 0xFFFFFFFF
			};


			array_push(mesh->faces, face);
		}

		//printf("LINE=%s", line);
	}

	array_free(texcoords);
	array_free(normals);
}


void load_mesh_png_data(mesh_t* mesh, char* filename)
{
	upng_t* png_image = upng_new_from_file(filename);
	if (png_image != NULL)
	{
		upng_decode(png_image);

		if (upng_get_error(png_image) == UPNG_EOK)
		{
			mesh->texture = png_image;
		}
	}
}

int get_num_meshes(void)
{
	return mesh_count;
}

mesh_t* get_mesh(int index)
{
	return &meshes[index];
}

void free_meshes(void)
{
	for (int i = 0; i < mesh_count; i++)
	{
		upng_free(meshes[i].texture);
		array_free(meshes[i].faces);
		array_free(meshes[i].vertices);
	}

}

