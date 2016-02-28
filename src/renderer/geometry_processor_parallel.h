// Copyright (c) 2012 Markus Trenkwalder

#ifndef GEOMETRYPROCESSOR_PARALLEL_H_
#define GEOMETRYPROCESSOR_PARALLEL_H_

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "irasterizer.h"
#include "vertex_processor.h"

#include <cstddef>

namespace swr {

class GeometryProcessorParallel {
public:
	typedef GeometryProcessor::CullMode CullMode;

	// make these inherited types and constants public
	typedef GeometryProcessor::VertexInput VertexInput;
	typedef GeometryProcessor::VertexOutput VertexOutput;

	static const int MAX_ATTRIBUTES = GeometryProcessor::MAX_ATTRIBUTES;

private:
	int thread_count_;
	std::vector<GeometryProcessor> geometry_processors_;

public:
	GeometryProcessorParallel() : thread_count_(4) {}

	void rasterizer(IRasterizer *r); // not implemented

	void addRasterizer(IRasterizer *r)
	{
		geometry_processors_.push_back(GeometryProcessor(r));
	}

	void thread_count(int n) { thread_count_ = n; }
	int thread_count() const { return thread_count_; }

	void viewport(int x, int y, int w, int h)
	{
		for (size_t i = 0; i < geometry_processors_.size(); ++i)
			geometry_processors_[i].viewport(x, y, w, h);
	}

	void depth_range(int n, int f)
	{
		for (size_t i = 0; i < geometry_processors_.size(); ++i)
			geometry_processors_[i].depth_range(n, f);
	}

	void vertex_attrib_pointer(int n, int stride, const void* buffer)
	{
		for (size_t i = 0; i < geometry_processors_.size(); ++i)
			geometry_processors_[i].vertex_attrib_pointer(n, stride, buffer);
	}

	void draw_triangles(unsigned count, unsigned *indices)
	{
		#pragma omp parallel for num_threads(thread_count_)
		for (int i = 0; i < (int) geometry_processors_.size(); ++i)
			geometry_processors_[i].draw_triangles(count, indices);
	}

	void draw_lines(unsigned count, unsigned *indices)
	{
		#pragma omp parallel for num_threads(thread_count_)
		for (int i = 0; i < (int) geometry_processors_.size(); ++i)
			geometry_processors_[i].draw_lines(count, indices);
	}

	void draw_points(unsigned count, unsigned *indices)
	{
		#pragma omp parallel for num_threads(thread_count_)
		for (int i = 0; i < (int) geometry_processors_.size(); ++i)
			geometry_processors_[i].draw_points(count, indices);
	}

	void cull_mode(CullMode m)
	{
		for (size_t i = 0; i < geometry_processors_.size(); ++i)
			geometry_processors_[i].cull_mode(m);
	}

	template<typename VertexShader>
	void vertex_shader()
	{
		for (size_t i = 0; i < geometry_processors_.size(); ++i)
			geometry_processors_[i].vertex_shader<VertexShader>();
	}
};

} // end namespace swr

#endif
