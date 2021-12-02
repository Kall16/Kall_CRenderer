#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vector.h"
#include "Texture.h"
#include "Swap.h"
#include <stdint.h>
#include "upng.h"

typedef struct
{
	vec4_t points[3];
	tex2_t texcoords[3];
	vec3_t normal;
	uint32_t color;
	upng_t* texture;
} triangle_t;

typedef struct
{
	int a;
	int b;
	int c;
	tex2_t a_uv;
	tex2_t b_uv;
	tex2_t c_uv;
	vec3_t face_normal;
	uint32_t color;
} face_t;

vec3_t get_triangle_normal(vec4_t vertices[3]);

void draw_filled_triangle(
	int x0, int y0, float z0, float w0,
	int x1, int y1, float z1, float w1,
	int x2, int y2, float z2, float w2,
	uint32_t color
);

void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

void draw_textured_triangle(
	int x0, int y0, float z0, float w0, float u0, float v0,
	int x1, int y1, float z1, float w1, float u1, float v1,
	int x2, int y2, float z2, float w2, float u2, float v2,
	upng_t* texture
);

void draw_texel(int x, int y, upng_t* texture,
	vec4_t point_a, vec4_t point_b, vec4_t point_c,
	tex2_t a_uv, tex2_t b_uv, tex2_t c_uv);

void draw_triangle_pixel(int x, int y, uint32_t color,
	vec4_t point_a, vec4_t point_b, vec4_t point_c);

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p);

#endif