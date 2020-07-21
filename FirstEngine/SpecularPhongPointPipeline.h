#pragma once

#include <cmath>
#include <memory>
#include <algorithm>

#include "ZBuffer.h"
#include "Triangle.h"
//#include "GraphicsClass.h"
#include "IndexedTriangleList.h"
#include "NDCScreenTransformer.h"
#include "SpecularPhongPointEffect.h"
#include "EngineOptions.h"

class GraphicsClass;

class SpecularPhongPointPipeline {
public:
	SpecularPhongPointPipeline(TextureClass& sysT);

	void Draw(IndexedTriangleList& triList);

	// needed to reset the z-buffer after each frame
	void BeginFrame();

private:

	// vertex processing function
	// transforms vertices using vs and then passes vtx & idx lists to triangle assembler
	void ProcessVertices(std::vector<Vertex>& vertices, std::vector<size_t>& indices);

	// triangle assembly function
	// assembles indexed vertex stream into triangles and passes them to post process
	// culls (does not send) back facing triangles
	void AssembleTriangles(std::vector<SpecularPhongPointEffect::VSOutput>& vertices, std::vector<size_t>& indices);

	// triangle processing function
	// passes 3 vertices to gs to generate triangle
	// sends generated triangle to post-processing
	void ProcessTriangle(SpecularPhongPointEffect::VSOutput& v0, SpecularPhongPointEffect::VSOutput& v1, SpecularPhongPointEffect::VSOutput& v2, size_t triangle_index);

	void ClipCullTriangle(Triangle<SpecularPhongPointEffect::VSOutput>& t);

	// vertex post-processing function
	// perform perspective and viewport transformations
	void PostProcessTriangleVertices(Triangle<SpecularPhongPointEffect::VSOutput>& triangle);

	// === triangle rasterization functions ===
	//   it0, it1, etc. stand for interpolants
	//   (values which are interpolated across a triangle in screen space)
	//
	// entry point for tri rasterization
	// sorts vertices, determines case, splits to flat tris, dispatches to flat tri funcs
	void DrawTriangle(Triangle<SpecularPhongPointEffect::VSOutput>& triangle);

	// does flat *TOP* tri-specific calculations and calls DrawFlatTriangle
	void DrawFlatTopTriangle(SpecularPhongPointEffect::VSOutput& it0, SpecularPhongPointEffect::VSOutput& it1, SpecularPhongPointEffect::VSOutput& it2);

	// does flat *BOTTOM* tri-specific calculations and calls DrawFlatTriangle
	void DrawFlatBottomTriangle(SpecularPhongPointEffect::VSOutput& it0, SpecularPhongPointEffect::VSOutput& it1, SpecularPhongPointEffect::VSOutput& it2);

	// does processing common to both flat top and flat bottom tris
	// scan over triangle in screen space, interpolate attributes,
	// depth cull, invoke ps and write pixel to screen
	void DrawFlatTriangle(SpecularPhongPointEffect::VSOutput& it0, SpecularPhongPointEffect::VSOutput& it1, SpecularPhongPointEffect::VSOutput& it2, SpecularPhongPointEffect::VSOutput& dv0, SpecularPhongPointEffect::VSOutput& dv1, SpecularPhongPointEffect::VSOutput itEdge1);
public:
	SpecularPhongPointEffect				effect;

private:
	
	std::shared_ptr<NDCScreenTransformer>	pst;
	std::shared_ptr<ZBuffer>				pZb;

	TextureClass&							mSysBuff;

	int										mWidth;
	int										mHeight;
};