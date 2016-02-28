// Copyright (c) 2012 Markus Trenkwalder

#ifndef RASTERIZER_PARALLEL_H
#define RASTERIZER_PARALLEL_H

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <vector>
#include "irasterizer.h"

namespace swr {

template <class SubRasterizer>
class RasterizerParallel: public IRasterizer {
private:
	std::vector<SubRasterizer> rasterizers_;
	int rows_;
	int cols_;
	int thread_count_;

public:
	RasterizerParallel(int rows, int cols, int thread_count = 4) :
			rows_(rows), cols_(cols), rasterizers_(rows * cols), thread_count_(thread_count)
	{
		perspective_correction(true);
		perspective_threshold(0, 0);
	}

public:
	void thread_count(int count)
	{ thread_count_ = count; }

	int thread_count() const
	{ return thread_count_; }

public:
	void perspective_correction(bool enable)
	{
		for (size_t i = 0; i < rasterizers_.size(); ++i)
			rasterizers_[i].perspective_correction(enable);
	}

	void perspective_threshold(int w, int h)
	{
		for (size_t i = 0; i < rasterizers_.size(); ++i)
			rasterizers_[i].perspective_threshold(w, h);
	}

	template<typename FragSpan>
	void fragment_shader()
	{
		for (size_t i = 0; i < rasterizers_.size(); ++i)
			rasterizers_[i].fragment_shader<FragSpan>();
	}

	void clip_rect(int x, int y, int w, int h)
	{
		int woffset = w / cols_;
		int hoffset = h / rows_;

		for (int r = 0; r < rows_; ++r) {
			for (int c = 0; c < cols_; ++c) {
				int index = r * cols_ + c;
				int clipwidth = c == cols_ - 1 ? w - c * woffset : woffset;
				int clipheight = r == cols_ - 1 ? h - r * hoffset : hoffset;
				rasterizers_[index].clip_rect(x + woffset * c, y + hoffset * r, clipwidth,
						clipheight);
			}
		}
	}

	void draw_triangle(const Vertex &v1, const Vertex &v2, const Vertex &v3)
	{
		for (size_t i = 0; i < rasterizers_.size(); ++i)
			rasterizers_[i].draw_triangle(v1, v2, v3);
	}

	void draw_line(const Vertex &v1, const Vertex &v2)
	{
		for (size_t i = 0; i < rasterizers_.size(); ++i)
			rasterizers_[i].draw_line(v1, v2);
	}

	void draw_point(const Vertex &v1)
	{
		for (size_t i = 0; i < rasterizers_.size(); ++i)
			rasterizers_[i].draw_point(v1);
	}

	void draw_triangle_list(const Vertex *vertices, const unsigned *indices, size_t index_count)
	{
		#pragma omp parallel for num_threads(thread_count_)
		for (int i = 0; i < (int) rasterizers_.size(); ++i)
			rasterizers_[i].draw_triangle_list(vertices, indices, index_count);
	}

	void draw_line_list(const Vertex *vertices, const unsigned *indices, size_t index_count)
	{
		#pragma omp parallel for num_threads(thread_count_)
		for (int i = 0; i < (int) rasterizers_.size(); ++i)
			rasterizers_[i].draw_line_list(vertices, indices, index_count);
	}

	void draw_point_list(const Vertex *vertices, const unsigned *indices, size_t index_count)
	{
		#pragma omp parallel for num_threads(thread_count_)
		for (int i = 0; i < (int) rasterizers_.size(); ++i)
			rasterizers_[i].draw_point_list(vertices, indices, index_count);
	}

	void userdata(void *userdata)
	{
		for (size_t i = 0; i < rasterizers_.size(); ++i)
			rasterizers_[i].userdata(userdata);
	}

	void* userdata()
	{
		return rasterizers_[0].userdata();
	}
};
} // end namespace swr

#endif
