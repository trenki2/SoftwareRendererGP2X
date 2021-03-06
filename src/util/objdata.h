/*
Copyright (c) 2007, 2008 Markus Trenkwalder

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

#ifndef OBJDATA_H_222A27C7_D005_417d_BE5C_3D7DB138806B
#define OBJDATA_H_222A27C7_D005_417d_BE5C_3D7DB138806B

#include "vector_math.h"
#include <vector>

// Use this class to load Obj files from disk 
// and convert them to vertex arrays.
struct ObjData {
	struct VertexArrayData {
		vmath::vec3<float> vertex;
		vmath::vec3<float> normal;
		vmath::vec2<float> texcoord;
	};

	struct VertexRef {
		unsigned vertex_index;
		unsigned normal_index;
		unsigned texcoord_index;
	};
	
	typedef std::vector<VertexRef> Face;

	std::vector< vmath::vec3<float> > vertices;
	std::vector< vmath::vec3<float> > normals;
	std::vector< vmath::vec2<float> > texcoords;
	std::vector<Face> faces;

	// Load the .obj file
	static ObjData load_from_file(const char *filename);

	// Convert to vertex and index array
	void to_vertex_array(std::vector<VertexArrayData> &vdata, std::vector<unsigned> &idata);
};

#endif
