#include "SpecularPhongPointPipeline.h"

#include "GraphicsClass.h"

SpecularPhongPointPipeline::SpecularPhongPointPipeline(const EngineOptions& options, std::shared_ptr<TextureClass> sysT) {

	mWidth		= options.m_screenWidth;
	mHeight		= options.m_screenHeight;
	mSysBuff	= sysT;
	pZb			= std::make_shared<ZBuffer>(mWidth, mHeight);
	pst			= std::make_shared<NDCScreenTransformer>(mWidth, mHeight);
}

void SpecularPhongPointPipeline::Draw(IndexedTriangleList& triList) {
	ProcessVertices(triList.vertices, triList.indices);
}

void SpecularPhongPointPipeline::BeginFrame() {
	pZb->Clear();
}

void SpecularPhongPointPipeline::ProcessVertices(std::vector<Vertex>& vertices, std::vector<size_t>& indices) {

	// create vertex vector for vs output
	std::vector<SpecularPhongPointEffect::VSOutput> verticesOut(vertices.size());

	// transform vertices with vs
	std::transform(vertices.begin(), vertices.end(), verticesOut.begin(), effect.vs);

	// assemble triangles from stream of indices and vertices
	AssembleTriangles(verticesOut, indices);
}

void SpecularPhongPointPipeline::AssembleTriangles(std::vector<SpecularPhongPointEffect::VSOutput>& vertices, std::vector<size_t>& indices) {

	//DirectX::XMVECTOR eyepos = DirectX::XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMLoadFloat4x4(&effect.vs.GetProj()));
	DirectX::XMFLOAT4X4& proj = effect.vs.GetProj();
	DirectX::XMVECTOR eyepos = DirectX::XMVectorSet(proj(3, 0), proj(3, 1), proj(3, 2), 1.0f);

	// assemble triangles in the stream and process
	for (size_t i = 0, end = indices.size() / 3; i < end; i++) {

		// determine triangle vertices via indexing
		SpecularPhongPointEffect::VSOutput& v0 = vertices[indices[i * 3]];
		SpecularPhongPointEffect::VSOutput& v1 = vertices[indices[i * 3 + 1]];
		SpecularPhongPointEffect::VSOutput& v2 = vertices[indices[i * 3 + 2]];

		DirectX::XMVECTOR vec1 = DirectX::operator-(DirectX::XMLoadFloat4(&v1.pos), DirectX::XMLoadFloat4(&v0.pos));
		DirectX::XMVECTOR vec2 = DirectX::operator-(DirectX::XMLoadFloat4(&v2.pos), DirectX::XMLoadFloat4(&v0.pos));
		DirectX::XMVECTOR norm = DirectX::XMVector3Cross(vec1, vec2);

		DirectX::XMVECTOR eye = DirectX::operator-(DirectX::XMLoadFloat4(&v2.pos), eyepos);

		// cull backfacing triangles with cross product (%) shenanigans
		if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(norm, eye)) <= 0.0f) {
			// process 3 vertices into a triangle
			ProcessTriangle(v0, v1, v2, i);
		}
	}
}

void SpecularPhongPointPipeline::ProcessTriangle(SpecularPhongPointEffect::VSOutput& v0, SpecularPhongPointEffect::VSOutput& v1, SpecularPhongPointEffect::VSOutput& v2, size_t triangle_index) {
	// generate triangle from 3 vertices using gs
	// and send to clipper
	Triangle<SpecularPhongPointEffect::VSOutput> t1 = effect.gs(v0, v1, v2, triangle_index);
	ClipCullTriangle(t1);
}

void SpecularPhongPointPipeline::ClipCullTriangle(Triangle<SpecularPhongPointEffect::VSOutput>& t) {

	// cull tests
	if (t.v0.pos.x > t.v0.pos.w &&
		t.v1.pos.x > t.v1.pos.w &&
		t.v2.pos.x > t.v2.pos.w) {
		return;
	}
	if (t.v0.pos.x < -t.v0.pos.w &&
		t.v1.pos.x < -t.v1.pos.w &&
		t.v2.pos.x < -t.v2.pos.w) {
		return;
	}
	if (t.v0.pos.y > t.v0.pos.w &&
		t.v1.pos.y > t.v1.pos.w &&
		t.v2.pos.y > t.v2.pos.w) {
		return;
	}
	if (t.v0.pos.y < -t.v0.pos.w &&
		t.v1.pos.y < -t.v1.pos.w &&
		t.v2.pos.y < -t.v2.pos.w) {
		return;
	}
	if (t.v0.pos.z > t.v0.pos.w &&
		t.v1.pos.z > t.v1.pos.w &&
		t.v2.pos.z > t.v2.pos.w) {
		return;
	}
	if (t.v0.pos.z < 0.0f &&
		t.v1.pos.z < 0.0f &&
		t.v2.pos.z < 0.0f) {
		return;
	}

	// clipping routines
	auto Clip1 = [this](SpecularPhongPointEffect::VSOutput& v0, SpecularPhongPointEffect::VSOutput& v1, SpecularPhongPointEffect::VSOutput& v2) {
		// calculate alpha values for getting adjusted vertices
		float alphaA = (-v0.pos.z) / (v1.pos.z - v0.pos.z);
		float alphaB = (-v0.pos.z) / (v2.pos.z - v0.pos.z);

		// interpolate to get v0a and v0b
		SpecularPhongPointEffect::VSOutput v0a = v0 + (v1 - v0) * alphaA;
		SpecularPhongPointEffect::VSOutput v0b = v0 + (v2 - v0) * alphaB;

		// draw triangles
		Triangle<SpecularPhongPointEffect::VSOutput> t1 = {v0a, v1, v2};
		Triangle<SpecularPhongPointEffect::VSOutput> t2 = {v0b, v0a, v2};
		PostProcessTriangleVertices(t1);
		PostProcessTriangleVertices(t2);
	};
	auto Clip2 = [this](SpecularPhongPointEffect::VSOutput& v0, SpecularPhongPointEffect::VSOutput& v1, SpecularPhongPointEffect::VSOutput& v2) {
		// calculate alpha values for getting adjusted vertices
		float alpha0 = (-v0.pos.z) / (v2.pos.z - v0.pos.z);
		float alpha1 = (-v1.pos.z) / (v2.pos.z - v1.pos.z);

		// interpolate to get v0a and v0b
		SpecularPhongPointEffect::VSOutput v0a = v0 + (v2 - v0) * alpha0;
		SpecularPhongPointEffect::VSOutput v1b = v1 + (v2 - v1) * alpha1;

		// draw triangles
		Triangle<SpecularPhongPointEffect::VSOutput> t1 = { v0a, v1b, v2 };
		PostProcessTriangleVertices(t1);
	};

	// near clipping tests
	if (t.v0.pos.z < 0.0f) {
		if (t.v1.pos.z < 0.0f) {
			Clip2(t.v0, t.v1, t.v2);
		}
		else if (t.v2.pos.z < 0.0f) {
			Clip2(t.v0, t.v2, t.v1);
		}
		else {
			Clip1(t.v0, t.v1, t.v2);
		}
	}
	else if (t.v1.pos.z < 0.0f) {
		if (t.v2.pos.z < 0.0f) {
			Clip2(t.v1, t.v2, t.v0);
		}
		else {
			Clip1(t.v1, t.v0, t.v2);
		}
	}
	else if (t.v2.pos.z < 0.0f) {
		Clip1(t.v2, t.v0, t.v1);
	}
	else {// no near clipping necessary
		PostProcessTriangleVertices(t);
	}
}

void SpecularPhongPointPipeline::PostProcessTriangleVertices(Triangle<SpecularPhongPointEffect::VSOutput>& triangle) {

	// perspective divide and screen transform for all 3 vertices
	pst->Transform(triangle.v0);
	pst->Transform(triangle.v1);
	pst->Transform(triangle.v2);

	// draw the triangle
	DrawTriangle(triangle);
}

void SpecularPhongPointPipeline::DrawTriangle(Triangle<SpecularPhongPointEffect::VSOutput>& triangle) {

	// using pointers so we can swap (for sorting purposes)
	SpecularPhongPointEffect::VSOutput* pv0 = &triangle.v0;
	SpecularPhongPointEffect::VSOutput* pv1 = &triangle.v1;
	SpecularPhongPointEffect::VSOutput* pv2 = &triangle.v2;

	// sorting vertices by y
	if (pv1->pos.y < pv0->pos.y) std::swap(pv0, pv1);
	if (pv2->pos.y < pv1->pos.y) std::swap(pv1, pv2);
	if (pv1->pos.y < pv0->pos.y) std::swap(pv0, pv1);

	// natural flat top
	if (pv0->pos.y == pv1->pos.y) {

		// sorting top vertices by x
		if (pv1->pos.x < pv0->pos.x) std::swap(pv0, pv1);
		DrawFlatTopTriangle(*pv0, *pv1, *pv2);
	}
	// natural flat bottom
	else if (pv1->pos.y == pv2->pos.y) {

		// sorting bottom vertices by x
		if (pv2->pos.x < pv1->pos.x) std::swap(pv1, pv2);
		DrawFlatBottomTriangle(*pv0, *pv1, *pv2);
	}
	// general triangle
	else {

		// find splitting vertex interpolant
		float alphaSplit = (pv1->pos.y - pv0->pos.y) / (pv2->pos.y - pv0->pos.y);
		SpecularPhongPointEffect::VSOutput vi = *pv0 + (*pv2 - *pv0) * alphaSplit;

		if (pv1->pos.x < vi.pos.x) // major right
		{
			DrawFlatBottomTriangle(*pv0, *pv1, vi);
			DrawFlatTopTriangle(*pv1, vi, *pv2);
		}
		else // major left
		{
			DrawFlatBottomTriangle(*pv0, vi, *pv1);
			DrawFlatTopTriangle(vi, *pv1, *pv2);
		}
	}
}

void SpecularPhongPointPipeline::DrawFlatTopTriangle(SpecularPhongPointEffect::VSOutput& it0, SpecularPhongPointEffect::VSOutput& it1, SpecularPhongPointEffect::VSOutput& it2) {
	// calulcate dVertex / dy
	// change in interpolant for every 1 change in y
	float delta_y = it2.pos.y - it0.pos.y;
	auto dit0 = (it2 - it0) / delta_y;
	auto dit1 = (it2 - it1) / delta_y;

	// create right edge interpolant
	auto itEdge1 = it1;

	// call the flat triangle render routine
	DrawFlatTriangle(it0, it1, it2, dit0, dit1, itEdge1);
}

void SpecularPhongPointPipeline::DrawFlatBottomTriangle(SpecularPhongPointEffect::VSOutput& it0, SpecularPhongPointEffect::VSOutput& it1, SpecularPhongPointEffect::VSOutput& it2) {
	// calulcate dVertex / dy
	// change in interpolant for every 1 change in y
	float delta_y = it2.pos.y - it0.pos.y;
	auto dit0 = (it1 - it0) / delta_y;
	auto dit1 = (it2 - it0) / delta_y;

	// create right edge interpolant
	auto itEdge1 = it0;

	// call the flat triangle render routine
	DrawFlatTriangle(it0, it1, it2, dit0, dit1, itEdge1);
}

void SpecularPhongPointPipeline::DrawFlatTriangle(SpecularPhongPointEffect::VSOutput& it0, SpecularPhongPointEffect::VSOutput& it1, SpecularPhongPointEffect::VSOutput& it2, SpecularPhongPointEffect::VSOutput& dv0, SpecularPhongPointEffect::VSOutput& dv1, SpecularPhongPointEffect::VSOutput itEdge1) {
	// create edge interpolant for left edge (always v0)
	auto itEdge0 = it0;

	// calculate start and end scanlines
	int yStart = std::max<float>((int)std::ceil(it0.pos.y - 0.5f), 0);
	int yEnd = std::min<float>((int)std::ceil(it2.pos.y - 0.5f), (int)mHeight - 1); // the scanline AFTER the last line drawn

	// do interpolant prestep
	itEdge0 += dv0 * (float(yStart) + 0.5f - it0.pos.y);
	itEdge1 += dv1 * (float(yStart) + 0.5f - it0.pos.y);

	for (int y = yStart; y < yEnd; y++, itEdge0 += dv0, itEdge1 += dv1) {

		// calculate start and end pixels
		int xStart = std::max<float>((int)std::ceil(itEdge0.pos.x - 0.5f), 0);
		int xEnd = std::min<float>((int)std::ceil(itEdge1.pos.x - 0.5f), (int)mWidth - 1); // the pixel AFTER the last pixel drawn

		// create scanline interpolant startpoint
		// (some waste for interpolating x,y,z, but makes life easier not having
		//  to split them off, and z will be needed in the future anyways...)
		auto iLine = itEdge0;

		// calculate delta scanline interpolant / dx
		float dx = itEdge1.pos.x - itEdge0.pos.x;
		auto diLine = (itEdge1 - iLine) / dx;

		// prestep scanline interpolant
		iLine += diLine * (float(xStart) + 0.5f - itEdge0.pos.x);

		for (int x = xStart; x < xEnd; x++, iLine += diLine) {
			// do z rejection / update of z buffer
			// skip shading step if z rejected (early z)
			if (pZb->TestAndSet(x, y, iLine.pos.z)) {
				// recover interpolated z from interpolated 1/z
				float w = 1.0f / iLine.pos.w;
				// recover interpolated attributes
				// (wasted effort in multiplying pos (x,y,z) here, but
				//  not a huge deal, not worth the code complication to fix)
				auto attr = iLine * w;
				// invoke pixel shader with interpolated vertex attributes
				// and use result to set the pixel color on the screen
				mSysBuff->PutPixel(x, y, effect.ps(attr));
			}
		}
	}
}