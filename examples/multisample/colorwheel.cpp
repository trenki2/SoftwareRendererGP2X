/*
Copyright (c) 2007, 2012 Markus Trenkwalder

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the library's copyright owner nor the names of its
  contributors may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SDL.h"
#undef main

// Requires software renderer source
#include "renderer/geometry_processor.h"
#include "renderer/rasterizer_subdivaffine.h"
#include "renderer/span.h"

#include "util/vector_math.h"

void *my_user_data = (void*) 0xDEADBEEF;

// the software renderer stuff is located in the namespace "swr" so include
// that here
using namespace swr;

// Our vertex structure which will be used to store our triangle data.
struct Vertex {
	float x, y;
	int r, g, b;
};

// This is the vertex shader which is executed for each individial vertex that
// needs to ne processed.
struct VertexShader {

	// This specifies that this shader is only going to use 1 vertex attribute
	// array. There you be used up to Renderer::MAX_ATTRIBUTES arrays.
	static const unsigned attribute_count = 1;

	// This specifies the number of varyings the shader will output. This is
	// for instance used when clipping.
	static const unsigned varying_count = 3;

	static vmath::mat4<float> transformation;

	// This static function is called for each vertex to be processed.
	// "in" is an array of void* pointers with the location of the individial
	// vertex attributes. The "out" structure has to be written to.
	static void shade(const GeometryProcessor::VertexInput in, GeometryProcessor::VertexOutput &out)
	{
		// cast the first attribute array to the input vertex type
		const Vertex &v = *static_cast<const Vertex*>(in[0]);

		vmath::vec4<float> r = transformation * vmath::vec4<float>(v.x, v.y, 0.0f, 1.0f);

		// x, y, z and w are the components that must be written by the vertex
		// shader. They all have to be specified in 16.16 fixed point format.
		out.x = static_cast<int>((r.x * (1 << 16)));
		out.y = static_cast<int>((r.y * (1 << 16)));
		out.z = 0;
		out.w = 1 << 16;

		// The vertexoutput can have up to Rasterizer::MAX_VARYING varying
		// parameters. These are just integer values which will be interpolated
		// across the primitives. The higher bits of these integers will be
		// interpolated more precicely so the values [0, 255] are shifted left.
		out.varyings[0] = v.r << 16;
		out.varyings[1] = v.g << 16;
		out.varyings[2] = v.b << 16;
	}
};

vmath::mat4<float> VertexShader::transformation = vmath::identity4<float>();

// This is the fragment shader (Multisampling 2x2)
struct FragmentShaderMultisampling : public SpanDrawerMultisampling<FragmentShaderMultisampling, 2> {
	// varying_count = 3 tells the rasterizer that it only needs to interpolate
	// three varying values (the r, g and b in this context).
	static const unsigned varying_count = 3;

	// We don't need to interpolate z in this example
	static const bool interpolate_z = false;

	// This static function will be called for every pixel that is to be
	// rasterized. x and y are the screen coordinates and fd hold the data for
	// the fragment that should be drawn
	static void single_fragment(
		int x,
		int y,
		const IRasterizer::FragmentData &fd,
		void *userdata,
		unsigned coverage_mask = 0xffffffff)
	{
		if (userdata != my_user_data)
			fprintf(stderr, "userdata pointer failure\n");

		SDL_Surface *screen = SDL_GetVideoSurface();
		unsigned short *color_buffer = static_cast<unsigned short*>(screen->pixels) + x * 2
				+ y * 2 * screen->w;

		// Convert from 16.16 color format to [0,255]
		// Here the colors are clamped to the range[0,255]. If this is not done
		// here we can get very small artifacts at the edges.
		int r = std::min(std::max(fd.varyings[0] >> 16, 0), 255);
		int g = std::min(std::max(fd.varyings[1] >> 16, 0), 255);
		int b = std::min(std::max(fd.varyings[2] >> 16, 0), 255);

		Uint32 color = SDL_MapRGB(screen->format, r, g, b);

		if (coverage_mask & 0x01)
			color_buffer[0] |= color;
		if (coverage_mask & 0x02)
			color_buffer[1] |= color;
		color_buffer += screen->pitch / 2;
		if (coverage_mask & 0x04)
			color_buffer[0] |= color;
		if (coverage_mask & 0x08)
			color_buffer[1] |= color;
	}
};

// This is the fragment shader
struct FragmentShaderStandard : public GenericSpanDrawer<FragmentShaderStandard> {
	// varying_count = 3 tells the rasterizer that it only needs to interpolate
	// three varying values (the r, g and b in this context).
	static const unsigned varying_count = 3;

	// We don't need to interpolate z in this example
	static const bool interpolate_z = false;

	// This static function will be called for every pixel that is to be
	// rasterized. x and y are the screen coordinates and fd hold the data for
	// the fragment that should be drawn
	static void single_fragment(
		int x,
		int y,
		const IRasterizer::FragmentData &fd,
		void *userdata,
		unsigned coverage_mask = 0xffffffff)
	{
		// Convert from 16.16 color format to [0,255]
		// Here the colors are clamped to the range[0,255]. If this is not done
		// here we can get very small artifacts at the edges.
		int r = std::min(std::max(fd.varyings[0] >> 16, 0), 255);
		int g = std::min(std::max(fd.varyings[1] >> 16, 0), 255);
		int b = std::min(std::max(fd.varyings[2] >> 16, 0), 255);

		// get the location of the pixel
		SDL_Surface *screen = SDL_GetVideoSurface();
		unsigned short *color_buffer = static_cast<unsigned short*>(screen->pixels) +
			x + y * screen->w;

		*color_buffer = SDL_MapRGB(screen->format, r, g, b);
	}
};

int main(int ac, char *av[]) {
	// Intialize SDL without error handling an all
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface *screen = SDL_SetVideoMode(512, 512, 16, 0);

	Vertex vertices[] = {
		{  0.00f,  0.00f, 255, 255, 255},
		{  0.50f,  0.00f, 255,   0,   0},
		{  0.25f,  0.43f, 255, 255,   0},
		{ -0.25f,  0.43f,   0, 255,   0},
		{ -0.50f,  0.00f,   0, 255, 255},
		{ -0.25f, -0.43f,   0,   0, 255},
		{  0.25f, -0.43f, 255,   0, 255},
	};

	unsigned indices[] = {
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 5,
		0, 5, 6,
		0, 6, 1
	};

	// Create a rasterizer class that will be used to rasterize primitives
	RasterizerSubdivAffine r;
	r.userdata(my_user_data);

	// Create a geometry processor class used to feed vertex data.
	GeometryProcessor g(&r);
	// It is necessary to set the viewport
	g.viewport(0, 0, screen->w * 2, screen->h * 2);

	// Set the cull mode (CW is already the default mode)
	g.cull_mode(GeometryProcessor::CULL_CW);

	// It is also necessary to set the clipping rectangle
	r.clip_rect(0, 0, screen->w * 2, screen->h * 2);

	// Set the vertex and fragment shaders
	g.vertex_shader<VertexShader>();
	r.fragment_shader<FragmentShaderMultisampling>();

	// Specify the vertex data
	g.vertex_attrib_pointer(0, sizeof(Vertex), vertices);

	r.perspective_correction(false);

	printf("Rendering time in ms (for 1000 frames)\n");
	printf("--------------------------------------\n");

	// Speed test without multisampling
	{
		r.fragment_shader<FragmentShaderStandard>();
		g.viewport(0, 0, screen->w / 2, screen->h / 2);
		r.clip_rect(0, 0, screen->w / 2, screen->h / 2);

		Uint32 t0 = SDL_GetTicks();
		for (int i = 0; i < 1000; ++i)
			g.draw_triangles(3 * 6, indices);
		Uint32 elapsed = SDL_GetTicks() - t0;
		printf("without multisampling: %i\n", elapsed);
	}

	// Speed test with supersampling
	{
		g.viewport(0, 0, screen->w, screen->h);
		r.clip_rect(0, 0, screen->w, screen->h);
		r.fragment_shader<FragmentShaderStandard>();

		Uint32 t0 = SDL_GetTicks();
		for (int i = 0; i < 1000; ++i)
			g.draw_triangles(3 * 6, indices);
		Uint32 elapsed = SDL_GetTicks() - t0;
		printf("with supersampling: %i\n", elapsed);
	}

	// Speed test with multisampling
	{
		g.viewport(0, 0, screen->w, screen->h);
		r.clip_rect(0, 0, screen->w, screen->h);
		r.fragment_shader<FragmentShaderMultisampling>();

		Uint32 t0 = SDL_GetTicks();
		for (int i = 0; i < 1000; ++i)
			g.draw_triangles(3 * 6, indices);
		Uint32 elapsed = SDL_GetTicks() - t0;
		printf("with multisampling: %i\n", elapsed);
	}

	g.viewport(0, 0, screen->w, screen->h);
	r.clip_rect(0, 0, screen->w, screen->h);
	r.fragment_shader<FragmentShaderMultisampling>();

	// Render an animation
	for (int i = 0; i < 360; ++i) {
		SDL_FillRect(SDL_GetVideoSurface(), 0, SDL_MapRGB(screen->format, 0, 0, 0));

		float factor = 0.002f;
		VertexShader::transformation = vmath::rotation_matrix((float) i, 0.0f, 0.0f, 1.0f)
			* vmath::translation_matrix(i * factor, 0.0f, 0.0f);

		g.draw_triangles(3 * 6, indices);

		// Show everything on screen
		SDL_Flip(SDL_GetVideoSurface());

		SDL_Delay(100);

		// Wait for the user closing the application
		SDL_Event e;
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT)
				goto end;
	}

	// Wait for the user closing the application
	SDL_Event e;
	while (SDL_WaitEvent(&e) && e.type != SDL_QUIT);

	end:
	// Quit SDL
	SDL_Quit();
	return 0;
}
