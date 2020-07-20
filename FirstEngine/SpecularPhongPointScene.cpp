#include "SpecularPhongPointScene.h"

#include "GraphicsClass.h"

SpecularPhongPointScene::SpecularPhongPointScene(const EngineOptions& options, std::shared_ptr<TextureClass> sysT, std::shared_ptr<TextureClass> wallT) : Scene("phong point shader scene free mesh") {

	pZb = std::make_shared<ZBuffer>(options.m_screenWidth, options.m_screenHeight);
	pipeline = std::make_shared<SpecularPhongPointPipeline>(options, sysT);

	// Set the initial position of the camera.
	m_Camera.SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera.SetLens(hfov, aspect_ratio, 1.0f, 1000.0f);

	// load walls
	DirectX::XMFLOAT4X4 wallWorld;
	DirectX::XMMATRIX wallTrans = DirectX::XMMatrixTranslation(0.0f, 0.0f, width / 2.0f);
	DirectX::XMStoreFloat4x4(&wallWorld, wallTrans);
	walls.push_back({
		wallT,
		Plane::GetSkinnedNormals(1, 1, width, width, tScaleWall),
		wallWorld
	});
}

void SpecularPhongPointScene::Update(float dt) {
	t += dt;

	m_Camera.UpdateViewMatrix();
}

void SpecularPhongPointScene::Draw() {

	pipeline->BeginFrame();

	DirectX::XMMATRIX view = m_Camera.GetViewMatrix();
	DirectX::XMVECTOR l_posXM = DirectX::XMLoadFloat4(&l_pos);
	DirectX::XMFLOAT4 lightPos(l_pos.x, l_pos.y, l_pos.z, 1.0f);
	DirectX::XMStoreFloat4(&lightPos, DirectX::XMVector4Transform(l_posXM, view));

	DirectX::XMFLOAT3 lp(lightPos.x, lightPos.y, lightPos.z);

	pipeline->effect.ps.SetLightPosition(lp);
	pipeline->effect.ps.SetAmbientLight(l_ambient);
	pipeline->effect.ps.SetDiffuseLight(l);

	DirectX::XMFLOAT4X4 prj = m_Camera.GetProjectionMatix4x4();
	for (auto& w : walls) {
		DirectX::XMFLOAT4X4 worldView;

		DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&w.world);
		DirectX::XMStoreFloat4x4(&worldView, DirectX::XMMatrixMultiply(world, view));

		pipeline->effect.vs.BindWorldView(worldView);
		pipeline->effect.vs.BindProjection(prj);
		pipeline->effect.ps.BindTexture(*w.pTex);
		pipeline->Draw(w.model);
	}
}

CameraClass& SpecularPhongPointScene::GetCamera() {
	return m_Camera;
};