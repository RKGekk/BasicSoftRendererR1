#pragma once

#include <DirectXMath.h>

#include "Scene.h"
#include "Wall.h"
#include "ZBuffer.h"
#include "SpecularPhongPointPipeline.h"
#include "ColorIntegers.h"
#include "IndexedTriangleList.h"
#include "Plane.h"
#include "CameraClass.h"

class GraphicsClass;

class SpecularPhongPointScene : public Scene {

public:

	struct Wall {
		TextureClass&		pTex;
		IndexedTriangleList	model;
		DirectX::XMFLOAT4X4	world;
	};

	SpecularPhongPointScene(TextureClass& sysT, TextureClass& wallT);

	virtual void Update(float dt) override;
	virtual void Draw() override;

	CameraClass& GetCamera();

private:
	float t = 0.0f;

	// scene params
	//static constexpr float width = 4.0f;
	static constexpr float width = 1.0f;
	static constexpr float height = 1.75f;

	// light
	DirectX::XMFLOAT4 l_pos		= { 0.0f,	0.0f,	0.0f,	-0.5f	};
	DirectX::XMFLOAT3 l			= { 1.0f,	1.0f,	1.0f	};
	DirectX::XMFLOAT3 l_ambient	= { 0.35f,	0.35f,	0.35f	};

	// fov
	//static constexpr float aspect_ratio	= 1.33333f;
	static constexpr float aspect_ratio = 1.0f;
	static constexpr float hfov			= 65.0f;
	static constexpr float vfov			= hfov / aspect_ratio;

	// camera stuff
	CameraClass m_Camera;

	// wall stuff
	static constexpr float tScaleCeiling = 0.5f;
	//static constexpr float tScaleWall = 0.65f;
	static constexpr float tScaleWall = 1.0f;
	static constexpr float tScaleFloor = 0.65f;

	std::vector<Wall> walls;

	// ripple stuff
	static constexpr float sauronSize = 0.6f;

	// pipelines
	std::shared_ptr<ZBuffer>					pZb;
	std::shared_ptr<SpecularPhongPointPipeline>	pipeline;
};