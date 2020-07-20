#include "GraphicsClass.h"

#include "memoryUtility.h"

GraphicsClass::GraphicsClass(const EngineOptions& options) {
	m_Direct3D		= nullptr;
	m_TextureHolder	= nullptr;
	m_TextureShader	= nullptr;
	m_Camera		= nullptr;
	m_Model			= nullptr;
	//m_Scene			= nullptr;
}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(const EngineOptions& options, HWND hwnd) {
	bool result;

	mClientWidth = options.m_screenWidth;
	mClientHeight = options.m_screenHeight;

	// Create the Direct3D object.
	m_Direct3D = std::shared_ptr<D3DClass>(new D3DClass);
	if (!m_Direct3D) {
		return false;
	}

	// Initialize the Direct3D object.
	result = m_Direct3D->Initialize(options, hwnd);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = std::shared_ptr<CameraClass>(new CameraClass);
	if (!m_Camera) {
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->SetLens(options.m_fov, AspectRatio(), 1.0f, 1000.0f);

	m_TextureHolder = std::shared_ptr<TextureHolder>(new TextureHolder);
	if (!m_TextureHolder) {
		return false;
	}

	// Initialize the texture holder object.
	std::vector<std::string> textures{ "stone01.tga", "stone02.tga", "10+Z.bmp" };
	result = m_TextureHolder->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), textures);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the textures.", L"Error", MB_OK);
		return false;
	}

	// Add blank soft texture
	m_TextureHolder->AddBlankSoftTexture(1024, 1024, "soft", ColorIntegers(0, 255, 0, 0));
	m_TextureHolder->GetTexture("soft").LoadToGPU(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	// Create the model object.
	m_Model = std::unique_ptr<ModelClass>(new ModelClass);
	if (!m_Model) {
		return false;
	}

	// Initialize the model object.
	result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_TextureHolder->GetTexture("10+Z.bmp"));
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Create the texture shader object.
	m_TextureShader = std::shared_ptr<TextureShaderClass>(new TextureShaderClass);
	if (!m_TextureShader) {
		return false;
	}

	// Initialize the color shader object.
	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	// Create scene object.
	m_Scene = std::shared_ptr<SpecularPhongPointScene>(new SpecularPhongPointScene(options, m_TextureHolder->GetSharedTexture("soft"), m_TextureHolder->GetSharedTexture("stone01.tga")));
	if (!m_Scene) {
		return false;
	}

	return true;
}

void GraphicsClass::Shutdown() {

	// Release the Direct3D object.
	if (m_Direct3D) {
		m_Direct3D->Shutdown();
	}

	return;
}

void GraphicsClass::OnResize(const EngineOptions& options) {

	mClientWidth = options.m_screenWidth;
	mClientHeight = options.m_screenHeight;

	m_Camera->SetLens(options.m_fov, AspectRatio(), 1.0f, 1000.0f);

	m_Direct3D->OnResize(options);
}

void GraphicsClass::Update(const GameTimer& gt) {

	//m_Model->Update(gt);
	m_Camera->UpdateViewMatrix();
	TextureClass& softTexture = m_TextureHolder->GetTexture("soft");
	softTexture.DrawLine(0, 0, 512, 512, { 255, 0, 0, 0 });
	softTexture.Update(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext());

	m_Scene->Update(gt.DeltaTime());
}

bool GraphicsClass::Frame() {
	bool result;

	m_Scene->Draw();

	// Render the graphics scene.
	result = Render();
	if (!result) {
		return false;
	}

	return true;
}

float GraphicsClass::AspectRatio() const {
	return static_cast<float>(mClientWidth) / mClientHeight;
}

std::shared_ptr<D3DClass> GraphicsClass::GetD3D() {
	return m_Direct3D;
}

std::shared_ptr<TextureHolder> GraphicsClass::GetTexture() {
	return m_TextureHolder;
}

std::shared_ptr<TextureShaderClass> GraphicsClass::GetShader() {
	return m_TextureShader;
}

std::shared_ptr<CameraClass> GraphicsClass::GetCamera() {
	return m_Camera;
}

int GraphicsClass::GetWidth() {
	return mClientWidth;
}

int GraphicsClass::GetHeight() {
	return mClientHeight;
}

bool GraphicsClass::Render() {
	
	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

	bool result;

	// Get the world, view, and projection matrices from the camera and d3d objects.
	XMMATRIX worldMatrix;
	m_Model->GetWorldMatrix(worldMatrix);

	XMMATRIX viewMatrix			= m_Camera->GetViewMatrix();
	XMMATRIX projectionMatrix	= m_Camera->GetProjectionMatix();

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// Render the model using the texture shader.
	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_TextureHolder->GetTexture("soft").GetTexture());
	//result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_TextureHolder->GetTexture("10+Z.bmp").GetTexture());
	if (!result) {
		return false;
	}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}
