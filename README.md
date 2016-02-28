# SoftwareRenderer
C++ Software Renderer developed for the GP2X ARM 200Mhz console.

This repository includes the sources for the renderer as well as the fixed point and vector math library as these are used in the examples.

If you only want the renderer core look in "src/renderer".

## Features
* Generic vertex arrays for arbitrary data in the vertex processing stage.
* Internal vertex cache for better vertex processing.
* Affine and perspective correct texture coordinate interpolation.
* Vertex and pixel shaders written in C++ using some C++ template magic.

## Example Code
```c++
// Create a rasterizer class that will be used to rasterzie primitives.
RasterizerSubdivAffine r;

// Create a geometry processor class used to feed vertex data.
GeometryProcessor g(&r);

// It is necessary to set the viewport.
g.viewport(0, 0, screen->w, screen->h);

// Set the cull mode.
g.cull_mode(GeometryProcessor::CULL_CW);

// It is also necessary to set the clipping rectangle.
r.clip_rect(0, 0, screen->w, screen->h);

// Set the vertex and fragment shaders.
g.vertex_shader<MyVertexShader>();
r.fragment_shader<MyFragmentShader>();

// Set perspective correction mode.
r.perspective_correction(true);

// Specify vertex data
g.vertex_attrib_pointer(0, sizeof(MyVertex), &vdata[0]);

// Draw triangles
g.draw_triangles(idata.size(), &idata[0]);
```
