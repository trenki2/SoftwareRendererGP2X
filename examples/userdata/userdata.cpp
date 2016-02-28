// Copyright (c) 2012 Markus Trenkwalder

#include "SDL.h"
#undef main

// Requires software renderer source
#include "renderer/geometry_processor.h"
#include "renderer/rasterizer_subdivaffine.h"
#include "renderer/span.h"

#define _TIMESPEC_DEFINED // prevent compiler error on VS2015
#include <pthread.h>

using namespace swr;

struct Userdata {
	int r;
	int g;
	int b;

	SDL_Surface *buffer;
};

struct Vertex {
	float x, y;
	int r, g, b;
};

struct VertexShader {
	static const unsigned attribute_count = 1;
	static const unsigned varying_count = 3;

	static void shade(const GeometryProcessor::VertexInput in, GeometryProcessor::VertexOutput &out)
	{
		const Vertex &v = *static_cast<const Vertex*>(in[0]);

		out.x = static_cast<int>((v.x * (1 << 16)));
		out.y = static_cast<int>((v.y * (1 << 16)));
		out.z = 0;
		out.w = 1 << 16;

		out.varyings[0] = v.r << 16;
		out.varyings[1] = v.g << 16;
		out.varyings[2] = v.b << 16;
	}
};

#define USE_GENERIC_SPAN_DRAWER 1

struct FragmentShader : public SpanDrawer16BitColorAndDepth<FragmentShader> {
	static const unsigned varying_count = 3;
	static const bool interpolate_z = false;

	static void begin_triangle(
		const IRasterizer::Vertex& v1,
		const IRasterizer::Vertex& v2,
		const IRasterizer::Vertex& v3,
		int area2,
		void *userdata)
	{}

	static void single_fragment(
		const IRasterizer::FragmentData &fd,
		unsigned short &color,
		unsigned short &depth,
		void *userdata)
	{
		Userdata *ud = (Userdata*) userdata;

		int r = std::min(std::max(fd.varyings[0] >> 16, 0), 255);
		int g = std::min(std::max(fd.varyings[1] >> 16, 0), 255);
		int b = std::min(std::max(fd.varyings[2] >> 16, 0), 255);

		// Blending with constant color and 0.5 alpha
		r = (r + ud->r) >> 1;
		g = (g + ud->g) >> 1;
		b = (b + ud->b) >> 1;

		SDL_Surface *buffer = ud->buffer;
		color = SDL_MapRGB(buffer->format, r, g, b);
	}

	static void* color_pointer(int x, int y, void *userdata)
	{
		Userdata *ud = (Userdata*) userdata;
		SDL_Surface *buffer = ud->buffer;
		return static_cast<unsigned short*>(buffer->pixels) + x + y * buffer->w;
	}

	static void* depth_pointer(int x, int y, void *userdata)
	{
		return 0;
	}
};

void* thread_function(void *arg)
{
	Userdata *userdata = static_cast<Userdata*>(arg);

	Vertex vertices[] = {
		{0.0f, 0.5f, 255, 0, 0},
		{-.5f, -.5f, 0, 255, 0},
		{0.5f, -.5f, 0, 0, 255},
	};

	unsigned indices[] = { 0, 1, 2 };

	RasterizerSubdivAffine r;
	GeometryProcessor g(&r);
	r.userdata(userdata);
	r.clip_rect(0, 0, userdata->buffer->w, userdata->buffer->h);
	r.fragment_shader<FragmentShader>();

	g.viewport(0, 0, userdata->buffer->w, userdata->buffer->h);
	g.cull_mode(GeometryProcessor::CULL_CW);
	g.vertex_attrib_pointer(0, sizeof(Vertex), vertices);
	g.vertex_shader<VertexShader>();

	g.draw_triangles(3, indices);

	return 0;
}

int main(int ac, char *av[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface *screen = SDL_SetVideoMode(640, 480, 16, 0);

	// Allocate separate data and buffers for each rendering thread.
	Userdata userdata1;
	Userdata userdata2;

	userdata1.r = 0;
	userdata1.g = 0;
	userdata1.b = 255;

	userdata2.r = 0;
	userdata2.g = 255;
	userdata2.b = 0;

	userdata1.buffer = SDL_DisplayFormat(screen);
	userdata2.buffer = SDL_DisplayFormat(screen);

	// Render separate frames using two different threads
	pthread_t thread1;
	pthread_t thread2;

	pthread_create(&thread1, 0, thread_function, &userdata1);
	pthread_create(&thread2, 0, thread_function, &userdata2);
	pthread_join(thread1, 0);
	pthread_join(thread2, 0);

	// Display half of each rendered buffer on the screen
	SDL_Rect left_rect;
	left_rect.x = 0;
	left_rect.y = 0;
	left_rect.w = 320;
	left_rect.h = 480;

	SDL_Rect right_rect;
	right_rect.x = 320;
	right_rect.y = 0;
	right_rect.w = 320;
	right_rect.h = 480;

	SDL_BlitSurface(userdata1.buffer, &left_rect, screen, &left_rect);
	SDL_BlitSurface(userdata2.buffer, &right_rect, screen, &right_rect);

	SDL_Flip(SDL_GetVideoSurface());

	SDL_Event e;
	while (SDL_WaitEvent(&e) && e.type != SDL_QUIT)
		;

	SDL_Quit();
	return 0;
}
