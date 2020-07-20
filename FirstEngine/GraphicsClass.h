#pragma once

#include <Windows.h>
#include <memory>

#include "D3DClass.h"
#include "EngineOptions.h"
#include "TextureHolder.h"
#include "TextureShaderClass.h"
#include "CameraClass.h"
#include "GameTimer.h"
#include "ModelClass.h"
#include "GDIPlusManager.h"
//#include "SpecularPhongPointPipeline.h"
#include "SpecularPhongPointScene.h"


class GraphicsClass {
public:
	GraphicsClass(const EngineOptions& options);
	GraphicsClass(const GraphicsClass&) = delete;
	GraphicsClass& operator=(const GraphicsClass&) = delete;
	~GraphicsClass();

	bool Initialize(const EngineOptions& options, HWND hwnd);
	void Shutdown();

	void OnResize(const EngineOptions& options);

	void Update(const GameTimer&);
	bool Frame();

	float AspectRatio() const;

	std::shared_ptr<D3DClass>			GetD3D();
	std::shared_ptr<TextureHolder>		GetTexture();
	std::shared_ptr<TextureShaderClass>	GetShader();
	std::shared_ptr<CameraClass>		GetCamera();

	int GetWidth();
	int GetHeight();

private:
	bool Render();

	GDIPlusManager								gdipMan;

	std::shared_ptr<D3DClass>					m_Direct3D;
	std::shared_ptr<TextureHolder>				m_TextureHolder;
	std::shared_ptr<TextureShaderClass>			m_TextureShader;
	std::shared_ptr<CameraClass>				m_Camera;
	std::unique_ptr<ModelClass>					m_Model;
	std::shared_ptr<SpecularPhongPointScene>	m_Scene;
	//std::shared_ptr<SpecularPhongPointPipeline>	m_Pipeline;
	

	int											mClientWidth;
	int											mClientHeight;
};