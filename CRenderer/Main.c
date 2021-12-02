#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "upng.h"
#include "Array.h"
#include "Display.h"
#include "Matrix.h"
#include "Vector.h"
#include "Light.h"
#include "Texture.h"
#include "Triangle.h"
#include "Mesh.h"
#include "Camera.h"
#include "Clipping.h"

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

bool is_running = false;
int previous_frame_time = 0;
float delta_time;

mat4_t world_matrix;
mat4_t proj_matrix; 
mat4_t view_matrix; 

void setup(void)
{
	set_render_method(RENDER_WIRE);
	set_cull_method(CULL_BACKFACE);

	// Initialize the scene light direction
	init_light(vec3_new(0, -1, 0));

	// Initialize the perspective projection matrix
	float aspect_x = (float)get_window_width() / (float)get_window_height();
	float aspect_y = (float)get_window_height() / (float)get_window_width();
	float fov_y = M_PI / 3.0f; // 180°/3 == 60°
	float fov_x = (atan(tan(fov_y / 2) * aspect_x)) * 2; // 180°/3 == 60°
	float z_near = 1.0f;
	float z_far = 100.0f;
	proj_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);

	// Initialize frustum planes with a point and a normal
	init_frustum_planes(fov_x, fov_y, z_near, z_far);

	load_mesh("./mesh/PC.obj", "./texture/PC.png", vec3_new(1, 1, 1), vec3_new(0, 0, 3), vec3_new(0, 0, 0));
	//load_mesh("./mesh/efa.obj", "./texture/efa.png", vec3_new(1, 1, 1), vec3_new(3, 0, 8), vec3_new(0, 0, 0));

}

void process_input()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{

		switch (event.type)
		{
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				is_running = false;
				break;
			}
			if (event.key.keysym.sym == SDLK_1)
			{
				set_render_method(RENDER_WIRE_VERTEX);
				break;
			}
			if (event.key.keysym.sym == SDLK_2)
			{
				set_render_method(RENDER_WIRE);
				break;
			}
			if (event.key.keysym.sym == SDLK_3)
			{
				set_render_method(RENDER_FILL_TRIANGLE);
				break;
			}
			if (event.key.keysym.sym == SDLK_4)
			{
				set_render_method(RENDER_FILL_TRIANGLE_WIRE);
				break;
			}
			if (event.key.keysym.sym == SDLK_5)
			{
				set_render_method(RENDER_TEXTURED);
				break;
			}
			if (event.key.keysym.sym == SDLK_6)
			{
				set_render_method(RENDER_TEXTURED_WIRE);
				break;
			}
			if (event.key.keysym.sym == SDLK_c)
			{
				set_cull_method(CULL_BACKFACE);
				break;
			}
			if (event.key.keysym.sym == SDLK_v)
			{
				set_cull_method(CULL_NONE);
				break;
			}
			if (event.key.keysym.sym == SDLK_z)
			{
				rotate_camera_pitch(+3.0 * delta_time);
				break;
			}
			if (event.key.keysym.sym == SDLK_s)
			{
				rotate_camera_pitch(-3.0 * delta_time);
				break;
			}
			if (event.key.keysym.sym == SDLK_RIGHT)
			{
				rotate_camera_yaw(+3.0 * delta_time);
				break;
			}
			if (event.key.keysym.sym == SDLK_LEFT)
			{
				rotate_camera_yaw(-3.0 * delta_time);
				break;
			}
			if (event.key.keysym.sym == SDLK_UP)
			{
				update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
				update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
				break;
			}
			if (event.key.keysym.sym == SDLK_DOWN)
			{
				update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
				update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
				break;
			}
			break;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Process the graphics pipeline stages for all the mesh triangles
///////////////////////////////////////////////////////////////////////////////
// +-------------+
// | Model space |  <-- original mesh vertices
// +-------------+
// |   +-------------+
// `-> | World space |  <-- multiply by world matrix
//     +-------------+
//     |   +--------------+
//     `-> | Camera space |  <-- multiply by view matrix
//         +--------------+
//         |    +------------+
//         `--> |  Clipping  |  <-- clip against the six frustum planes
//              +------------+
//              |    +------------+
//              `--> | Projection |  <-- multiply by projection matrix
//                   +------------+
//                   |    +-------------+
//                   `--> | Image space |  <-- apply perspective divide
//                        +-------------+
//                        |    +--------------+
//                        `--> | Screen space |  <-- ready to render
//                             +--------------+
///////////////////////////////////////////////////////////////////////////////
void process_graphics_pipeline_stages(mesh_t* mesh)
{

	// Create a scale matrix that will be used to multiply the mesh vertices
	mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
	mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

	// Create the view matrix looking at a hardcoded target point
	// Compute the new camera rotation and translation for the FPS camera movement

	// Offset tha camera position in the direction where the camera is pointing at
	vec3_t target = get_camera_lookat_target();
	vec3_t up_direction = vec3_new(0, 1, 0);
	view_matrix = mat4_look_at(get_camera_position(), target, up_direction);


	// Loop all triangle faces of our mesh
	int num_faces = array_length(mesh->faces);
	for (int i = 0; i < num_faces; i++)
	{
		face_t mesh_face = mesh->faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh->vertices[mesh_face.a];
		face_vertices[1] = mesh->vertices[mesh_face.b];
		face_vertices[2] = mesh->vertices[mesh_face.c];

		vec4_t transformed_vertices[3];

		//Loop all three vertices of this current face and apply transformation
		for (int j = 0; j < 3; j++)
		{
			vec4_t  transformed_vertex = vec4_from_vec3(face_vertices[j]);

			// Create a world matrix combining scale, rotation, translation
			world_matrix = mat4_identity();

			// Order matters : First Scale, then rotate, then translate 
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

			// https://waynewolf.github.io/2013/05/30/transform-normal-to-eye-space/
			// Multiply the view matrux by the vector to transform the scene to camera space
			transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

			//Save transformed vertices
			transformed_vertices[j] = transformed_vertex;
		}

		///////////////////////////////////////////////////////////////Check Backface Culling

		//// normal matrix is the transpose of inverse model-view matrix
		vec3_t face_normal = get_triangle_normal(transformed_vertices);
		//face_normal = vec3_from_vec4(mat4_mul_vec4(view_matrix, vec4_from_vec3(mesh_face.face_normal)));


		if (is_cull_backface())
		{
			//Find a vector between a point in the triangle and the camera origin
			vec3_t camera_ray = vec3_sub(vec3_new(0, 0, 0), vec3_from_vec4(transformed_vertices[0]));

			// Calculate how aligned is the camera ray with the normal
			float dot_normal_camera = vec3_dot(camera_ray, face_normal);

			if (dot_normal_camera < 0)
			{
				continue;
			}

		}
		//-- end of back face culling

		// CLIPPING
		//Create a polygon from the original transform 
		polygon_t polygon = create_polygon_from_triangle(
			vec3_from_vec4(transformed_vertices[0]),
			vec3_from_vec4(transformed_vertices[1]),
			vec3_from_vec4(transformed_vertices[2]),
			mesh_face.a_uv,
			mesh_face.b_uv,
			mesh_face.c_uv,
			face_normal
		);

		// Clip the polygon and returns a new polygon
		clip_polygon(&polygon);

		// After clippng, we need to break polygon into triangle
		triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
		int num_triangles_afer_clipping = 0;

		triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_afer_clipping);

		// Loops all the assembled triangles after clipping

		for (int t = 0; t < num_triangles_afer_clipping; t++)
		{
			triangle_t triangle_after_clipping = triangles_after_clipping[t];

			vec4_t projected_points[3];

			// Loop all three vertices to perform projection
			for (int j = 0; j < 3; j++)
			{
				// Project the current vertex
				projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

				// Scale into the view
				projected_points[j].x *= (get_window_width() / 2.0f);
				projected_points[j].y *= (get_window_height() / 2.0f);

				// Invert the y values to account for flipped screen y coordinate
				projected_points[j].y *= -1;

				//Translate the projected point in the middle of the screen
				projected_points[j].x += (get_window_width() / 2.0f);
				projected_points[j].y += (get_window_height() / 2.0f);

			}

			//Calculate the shade intensity based on how aligned is the face normal aligned to the light direction
			float light_intensity_factor = -vec3_dot(face_normal, get_light_direction());

			uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

			triangle_t triangle_to_render =
			{
				.points =
				{
					{projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
					{projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
					{projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w}
				},
				.texcoords =
				{
					{triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v},
					{triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v},
					{triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v}
				},
				.color = triangle_color,
				.texture = mesh->texture,
				.normal = triangle_after_clipping.normal
			};

			// Save the projected triangle in the array of triangle to render
			//triangles_to_render[i] = projected_triangle;
			if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
			{
				triangles_to_render[num_triangles_to_render] = triangle_to_render;
				num_triangles_to_render++;
			}
		}
	}
}

void update()
{
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

	previous_frame_time = SDL_GetTicks();

	// Initilaze the counter of triangle to render for the current frame
	num_triangles_to_render = 0;

	// Loop all the meshes of our scene
	for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++)
	{
		mesh_t* mesh = get_mesh(mesh_index);


		// Change the mesh scale, rotation values per animation frames
		//mesh->rotation.x += 0.5f * delta_time;
		mesh->rotation.y += 0.5f * delta_time;
		//mesh->rotation.z += 0.1f * delta_time;
		//mesh.scale.x += 0.002f * delta_time;
		//mesh.scale.y += 0.001f * delta_time;
		//mesh.translation.x += 0.01;

		// Change the camera positon per aniamtion frame
		/*camera.position.x += 0.8 * delta_time;
		camera.position.y += 0.8 * delta_time;*/

		process_graphics_pipeline_stages(mesh);
	}
}


void render()
{
	clear_color_buffer(0xFF000000);
	clear_z_buffer();
	draw_grid(10);

	for (int i = 0; i < num_triangles_to_render; i++)
	{
		triangle_t triangle = triangles_to_render[i];
		
		if (should_render_filled_triangle())
		{
			draw_filled_triangle(
				triangle.points[0].x,
				triangle.points[0].y,
				triangle.points[0].z,
				triangle.points[0].w,
				triangle.points[1].x,
				triangle.points[1].y,
				triangle.points[1].z,
				triangle.points[1].w,
				triangle.points[2].x,
				triangle.points[2].y,
				triangle.points[2].z,
				triangle.points[2].w,
				triangle.color
			);
		}

		// Draw texture triangle
		if (should_render_textured_triangle())
		{
			draw_textured_triangle(
				triangle.points[0].x,
				triangle.points[0].y,
				triangle.points[0].z,
				triangle.points[0].w,
				triangle.texcoords[0].u,
				triangle.texcoords[0].v,
				triangle.points[1].x,
				triangle.points[1].y,
				triangle.points[1].z,
				triangle.points[1].w,
				triangle.texcoords[1].u,
				triangle.texcoords[1].v,
				triangle.points[2].x,
				triangle.points[2].y,
				triangle.points[2].z,
				triangle.points[2].w,
				triangle.texcoords[2].u,
				triangle.texcoords[2].v,
				triangle.texture
			);
		}

		if (should_render_wireframe())
		{
			draw_triangle(
				triangle.points[0].x,
				triangle.points[0].y,
				triangle.points[1].x,
				triangle.points[1].y,
				triangle.points[2].x,
				triangle.points[2].y,
				0xFFFFFFFF
			);
		}

		if (should_render_vertex())
		{
			// Draw Vertices
			draw_rect(triangle.points[0].x - 2, triangle.points[0].y - 2, 4, 4, 0xFFFF0000);
			draw_rect(triangle.points[1].x - 2, triangle.points[1].y - 2, 4, 4, 0xFFFF0000);
			draw_rect(triangle.points[2].x - 2, triangle.points[2].y - 2, 4, 4, 0xFFFF0000);
		}
	}


	render_color_buffer();

}

//Free memory that was dynamically allocated by the program
void free_resources(void)
{
	free_meshes();
	destroy_window();
}

int main(int argc, char* args[])
{
	/* Create SDL Window */
	is_running = initialize_window();
	
	setup();

	while (is_running)
	{
		process_input();
		update();
		render();
	}

	free_resources();

	return 0; 
}