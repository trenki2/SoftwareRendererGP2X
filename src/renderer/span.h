/*
Copyright (c) 2007, 2008, 2012 Markus Trenkwalder

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


#ifndef SPAN_5C49406D_561D_4dd1_85B0_9EBA56E58CB2
#define SPAN_5C49406D_561D_4dd1_85B0_9EBA56E58CB2

#ifdef _MSC_VER
#pragma once
#endif

#include "irasterizer.h"
#include "duffsdevice.h"
#include "fixed_func.h"
#include "util.h"

#include "stepmacros.h"
#include <vector>
#include <limits>

namespace swr {

	template <typename FragmentShader>
	struct SpanDrawerBase {
		static const int AFFINE_LENGTH = 24;

		static IRasterizer::FragmentData compute_step_al(
			const IRasterizer::FragmentData &fdl, 
			const IRasterizer::FragmentData &fdr)
		{
			IRasterizer::FragmentData r;

			if (FragmentShader::interpolate_z)
				r.z = (fdr.z - fdl.z) / AFFINE_LENGTH;

			DUFFS_DEVICE8(
				int i = 0,
				r.varyings[i] = (fdr.varyings[i] - fdl.varyings[i]) / AFFINE_LENGTH; ++i,
				FragmentShader::varying_count,
				/**/)

			return r;
		}

		static IRasterizer::FragmentData compute_step(
			const IRasterizer::FragmentData &fdl, 
			const IRasterizer::FragmentData &fdr,
			int inv_delta)
		{
			using namespace detail;
			
			IRasterizer::FragmentData r;

			if (FragmentShader::interpolate_z)
				r.z = fixmul<16>(fdr.z - fdl.z, inv_delta);

			DUFFS_DEVICE8(
				int i = 0,
				r.varyings[i] = fixmul<16>(fdr.varyings[i] - fdl.varyings[i], inv_delta); ++i,
				FragmentShader::varying_count,
				/**/)

			return r;
		}

		static IRasterizer::FragmentData fd_from_fds(const IRasterizer::FragmentDataPerspective &fd)
		{
			using namespace detail;
			
			IRasterizer::FragmentData r = IRasterizer::FragmentData();

			if (FragmentShader::interpolate_z)
				r.z = fd.fd.z;

			if (FragmentShader::varying_count) {
				int w = detail::invert(fd.oow);

				DUFFS_DEVICE8(
					int i = 0,
					r.varyings[i] = fixmul<16>(fd.fd.varyings[i], w); ++i,
					FragmentShader::varying_count,
					/**/)
			}

			return r;
		}

		static void perspective_span(
			int x, 
			int y, 
			const IRasterizer::FragmentDataPerspective &fd_in, 
			const IRasterizer::FragmentDataPerspective &step, 
			unsigned n,
			void *userdata)
		{
			using namespace detail;

			IRasterizer::FragmentDataPerspective fds[2];
			FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[0], = , fd_in);

			IRasterizer::FragmentData fd[2];
			fd[0] = fd_from_fds(fds[0]);

			while (AFFINE_LENGTH <= static_cast<int>(n)) {
				FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[1], = , fds[0]);
				FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[1], += AFFINE_LENGTH *, step);

				fd[1] = fd_from_fds(fds[1]);

				const IRasterizer::FragmentData step = compute_step_al(fd[0], fd[1]);

				FragmentShader::affine_span(x, y, fd[0], step, AFFINE_LENGTH, userdata);
				x += AFFINE_LENGTH; n -= AFFINE_LENGTH;

				FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[0], =, fds[1]);
				FRAGMENTDATA_APPLY(FragmentShader, fd[0], =, fd[1]);
			}

			if (n) {
				const int inv_n = detail::invert(n << 16);

				FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[1], = , fds[0]);
				FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[1], += n *, step);

				fd[1] = fd_from_fds(fds[1]);

				const IRasterizer::FragmentData step = compute_step(fd[0], fd[1], inv_n);

				FragmentShader::affine_span(x, y, fd[0], step, n, userdata);
			}
		}

		// Per triangle callback. This could for instance be used to select the
		// mipmap level of detail. Empty function defined here, so that it it optional for the
		// fragment shader.
		static void begin_triangle(
			const IRasterizer::Vertex& v1,
			const IRasterizer::Vertex& v2,
			const IRasterizer::Vertex& v3,
			int area2,
			void *userdata)
		{}

		// Called when the Rasterizer finishes the current triangle
		static void end_triangle(
			const IRasterizer::Vertex& v1,
			const IRasterizer::Vertex& v2,
			const IRasterizer::Vertex& v3,
			void *userdata)
		{}

		// Line callback
		static void begin_line(
			const IRasterizer::Vertex &v1,
			const IRasterizer::Vertex &v2)
		{}

		static void end_line(
			const IRasterizer::Vertex &v1,
			const IRasterizer::Vertex &v2)
		{}
	};

	template <typename FragmentShader>
	struct GenericSpanDrawer : public SpanDrawerBase<FragmentShader> {
		static void affine_span(
			int x, 
			int y, 
			IRasterizer::FragmentData fd, 
			const IRasterizer::FragmentData &step, 
			unsigned n,
			void *userdata)
		{
			DUFFS_DEVICE8(
				/**/,
				FragmentShader::single_fragment(x++, y, fd, userdata);
				FRAGMENTDATA_APPLY(FragmentShader, fd, +=, step),
				n,
				/**/)
		}
	};

	template <typename FragmentShader>
	struct SpanDrawer16BitColorAndDepth : public SpanDrawerBase<FragmentShader> {
		static void affine_span(
			int x, 
			int y, 
			IRasterizer::FragmentData fd, 
			const IRasterizer::FragmentData &step, 
			unsigned n,
			void *userdata)
		{
		unsigned short *color16_pointer =
				static_cast<unsigned short*>(FragmentShader::color_pointer(x, y, userdata));
		unsigned short *depth16_pointer =
				static_cast<unsigned short*>(FragmentShader::depth_pointer(x, y, userdata));

			// using duffs device for loop unrolling can improve performance
			// and seems useful even when using -funroll-loops in GCC.
			// In a few tests the above was 5% faster than a normal while loop with -funroll-loops.

			DUFFS_DEVICE16(
				/**/,
				{
					FragmentShader::single_fragment(fd, *color16_pointer, *depth16_pointer, userdata);
					FRAGMENTDATA_APPLY(FragmentShader, fd, += , step);

					color16_pointer++;
					depth16_pointer++;
				},
				n,
				/**/)
		}
	};

	template <typename FragmentShader, int SampleCount>
	struct SpanDrawerMultisampling: public SpanDrawerBase<FragmentShader> {
		struct SampleData {
			unsigned coverage_mask;
			IRasterizer::FragmentData fd;
			void *userdata;
		};

		static std::vector<SampleData>& span_buffer()
		{ static std::vector<SampleData> value(2048); return value; }

		static int& span_data_y()
		{ static int value = -1; return value; }

		static int& span_min_x()
		{ static int value = (std::numeric_limits<int>::max)(); return value; }

		static int& span_max_x()
		{ static int value; return value; }

		static int& last_span_y()
		{ static int value; return value; }

		static void initialize(int max_x_coord)
		{
			// make sure spanbuffer is large enough
			size_t maxx = detail::ceil28_4(max_x_coord);
			if (span_buffer().size() < maxx)
				span_buffer().resize(maxx);

			for (size_t i = 0; i < span_buffer().size(); ++i)
				span_buffer()[i].coverage_mask = 0;

			span_data_y() = -1;
			span_min_x() = (std::numeric_limits<int>::max)();
			span_max_x() = 0;
			last_span_y() = -1;
		}

		static void begin_span(int y)
		{
			// Do nothing
		}

		static void end_span(int y)
		{
			if (y % SampleCount == 0 && span_data_y() != -1) {
				emit_span_data();
				span_data_y() = -1;
				span_min_x() = (std::numeric_limits<int>::max)();
				span_max_x() = 0;
				last_span_y() = -1;
			}
		}

		static void detect_begin_end_span(int y)
		{
			if (last_span_y() != y) {
				if (span_data_y() != -1)
					end_span(y);
				last_span_y() = y;
				begin_span(y);
			}
		}

		static void begin_line(const IRasterizer::Vertex &v1, const IRasterizer::Vertex &v2)
		{
			int max_x_coord = (std::max)(v1.x, v2.x);
			initialize(max_x_coord / SampleCount + 1);
		}

		static void end_line(const IRasterizer::Vertex &v1, const IRasterizer::Vertex &v2)
		{
			if (span_data_y() != -1)
				emit_span_data();
		}

		static void begin_triangle(
			const IRasterizer::Vertex& v1,
			const IRasterizer::Vertex& v2,
			const IRasterizer::Vertex& v3,
			int area2,
			void *userdata)
		{
			int max_x_coord = (std::max)((std::max)(v1.x + v1.w, v2.x + v2.w), v3.x + v3.w);
			initialize(max_x_coord / SampleCount + 1);
		}

		static void end_triangle(
			const IRasterizer::Vertex& v1,
			const IRasterizer::Vertex& v2,
			const IRasterizer::Vertex& v3,
			void *userdata)
		{
			if (span_data_y() != -1)
				emit_span_data();
		}

		static void emit_span_data()
		{
			if (span_data_y() != -1) {
				for (int i = span_min_x(); i <= span_max_x(); ++i) {
					if (span_buffer()[i].coverage_mask) {
						FragmentShader::single_fragment(i, span_data_y(), span_buffer()[i].fd,
								span_buffer()[i].userdata, span_buffer()[i].coverage_mask);
						span_buffer()[i].coverage_mask = 0;
					}
				}
			}
		}

		static void affine_span(
			int x,
			int y,
			IRasterizer::FragmentData fd,
			const IRasterizer::FragmentData &step,
			unsigned in_n,
			void *userdata)
		{
			int n = in_n; // n is required as a signed integer in the computations

			detect_begin_end_span(y);

			span_data_y() = y / SampleCount;
			span_min_x() = (std::min)(span_min_x(), x / SampleCount);

			int span_mask = 0;
			for (int i = 0; i < SampleCount; ++i)
				span_mask |= (1 << i % SampleCount);

			int row_shift = (y % SampleCount) * SampleCount;
			int jumpstep = 0;

			while (x % SampleCount != 0) {
				if (span_buffer()[x / SampleCount].coverage_mask == 0) {
					span_buffer()[x / SampleCount].fd = fd;
					span_buffer()[x / SampleCount].userdata = userdata;
					FRAGMENTDATA_APPLY(FragmentShader, fd, +=, step);
				} else {
					jumpstep++;
				}

				span_buffer()[x / SampleCount].coverage_mask |= ((0x01 << (x % SampleCount))
						<< row_shift);

				x++;
				n--;
			}

			while (n >= SampleCount) {
				if (span_buffer()[x / SampleCount].coverage_mask == 0) {
					if (jumpstep) {
						FRAGMENTDATA_APPLY(FragmentShader, fd, += jumpstep *, step);
						jumpstep = 0;
					}
					span_buffer()[x / SampleCount].fd = fd;
					span_buffer()[x / SampleCount].userdata = userdata;
					FRAGMENTDATA_APPLY(FragmentShader, fd, += SampleCount *, step);
				} else {
					jumpstep += SampleCount;
				}

				span_buffer()[x / SampleCount].coverage_mask |= (span_mask << row_shift);

				x += SampleCount;
				n -= SampleCount;
			}

			while (n > 0) {
				if (span_buffer()[x / SampleCount].coverage_mask == 0) {
					if (jumpstep) {
						FRAGMENTDATA_APPLY(FragmentShader, fd, += jumpstep *, step);
						jumpstep = 0;
					}
					span_buffer()[x / SampleCount].fd = fd;
					span_buffer()[x / SampleCount].userdata = userdata;
					FRAGMENTDATA_APPLY(FragmentShader, fd, +=, step);
				}

				span_buffer()[x / SampleCount].coverage_mask |= ((0x01 << (x % SampleCount))
						<< row_shift);

				x++;
				n--;
			}

			span_max_x() = (std::max)(span_max_x(), x / SampleCount);
		}
	};
}

#include "stepmacros_undef.h"

#endif

