#include "frame_capture.h"

#include "renderdoc/renderdoc_app.h"
#include "platform.h"

#include <iostream>

#if defined(CM5_PLATFORM_WINDOWS)
#include <Windows.h>
#elif defined(CM5_PLATFORM_LINUX)
#include <dlfcn.h>
#elif defined(CM5_PLATFORM_MACOS)
#include <dlfcn.h>
#endif

static RENDERDOC_API_1_1_2* g_renderDocApi = nullptr;

void initFrameCapture()
{
	pRENDERDOC_GetAPI RENDERDOC_GetAPI = nullptr;

#if defined(CM5_PLATFORM_WINDOWS)
	if (HMODULE mod = LoadLibraryA("renderdoc.dll"))
	{
		RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
	}
#elif defined(CM5_PLATFORM_LINUX) || defined(CM5_PLATFORM_MACOS)
	if (void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_LOCAL))
	{
		RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
	}
#endif

	if (RENDERDOC_GetAPI && !RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&g_renderDocApi))
	{
		std::cerr << "Failed to acquire RenderDoc API functions" << std::endl;
		g_renderDocApi = nullptr;
	}

	if (g_renderDocApi)
	{
		g_renderDocApi->LaunchReplayUI(1, nullptr);
	}
}

bool isFrameCaptureSupported()
{
	return g_renderDocApi != nullptr;
}

void beginFrameCapture()
{
	if (!g_renderDocApi) return;

	g_renderDocApi->StartFrameCapture(nullptr, nullptr);
}

void endFrameCapture()
{
	if (!g_renderDocApi) return;

	g_renderDocApi->EndFrameCapture(nullptr, nullptr);
}

void launchFrameReplay()
{
	if (!g_renderDocApi) return;

	g_renderDocApi->LaunchReplayUI(1, nullptr);
}