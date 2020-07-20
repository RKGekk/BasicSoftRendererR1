#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <memory>

#include "inputclass.h"
#include "GameTimer.h"
#include "EngineOptions.h"
#include "GraphicsClass.h"

class SystemClass {
public:
	SystemClass();
	SystemClass(const SystemClass&) = delete;
	SystemClass& operator=(const SystemClass&) = delete;
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	void OnResize();

	static Renderer GetRendererType();
	static GraphicsClass& GetGraphics();
	static EngineOptions& GetEngineOptions();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);	

private:
	bool Frame();
	void Update(const GameTimer&);
	bool InitializeWindows();
	void ShutdownWindows();

	static SystemClass* m_applicationHandle;

	LPCWSTR			m_applicationName;
	HINSTANCE		m_hinstance;
	HWND			m_hwnd;

	bool			m_AppPaused;
	bool			m_Minimized;
	bool			m_Maximized;
	bool			m_Resizing;
	EngineOptions	m_engineOptions;

	GameTimer		m_Timer;
	InputClass		m_Input;

	GraphicsClass	m_Graphics;
};