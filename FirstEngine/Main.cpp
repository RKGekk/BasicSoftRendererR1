#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "gdiplus.lib" )

#include <iostream>
#include <memory>

#include "SystemClass.h"
#include <crtdbg.h>

int main() {

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::unique_ptr<SystemClass> System(new SystemClass);
	if (System->Initialize()) {
		System->Run();
	}
	// Shutdown and release the system object.
	System->Shutdown();
	
	return 0;
}