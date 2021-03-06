// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/core/App.h"
#include "anki/misc/ConfigSet.h"
#include "anki/util/Logger.h"
#include "anki/util/File.h"
#include "anki/util/Filesystem.h"
#include "anki/util/System.h"
#include "anki/core/Counters.h"

#include "anki/core/NativeWindow.h"
#include "anki/input/Input.h"
#include "anki/scene/SceneGraph.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/script/ScriptManager.h"

#include <signal.h>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

// Sybsystems

namespace anki {

#if ANKI_OS == ANKI_OS_ANDROID
/// The one and only android hack
android_app* gAndroidApp = nullptr;
#endif

//==============================================================================
App::App()
{}

//==============================================================================
App::~App()
{
	cleanup();
}

//==============================================================================
void App::cleanup()
{
	if(m_script != nullptr)
	{
		m_heapAlloc.deleteInstance(m_script);
		m_script = nullptr;
	}

	if(m_scene != nullptr)
	{
		m_heapAlloc.deleteInstance(m_scene);
		m_scene = nullptr;
	}

	if(m_renderer != nullptr)
	{
		m_heapAlloc.deleteInstance(m_renderer);
		m_renderer = nullptr;
	}

	if(m_resources != nullptr)
	{
		m_heapAlloc.deleteInstance(m_resources);
		m_resources = nullptr;
	}

	if(m_physics)
	{
		m_heapAlloc.deleteInstance(m_physics);
		m_physics = nullptr;
	}

	if(m_gr != nullptr)
	{
		m_heapAlloc.deleteInstance(m_gr);
		m_gr = nullptr;
	}

	if(m_threadpool != nullptr)
	{
		m_heapAlloc.deleteInstance(m_threadpool);
		m_threadpool = nullptr;
	}

	if(m_input != nullptr)
	{
		m_heapAlloc.deleteInstance(m_input);
		m_input = nullptr;
	}

	if(m_window != nullptr)
	{
		m_heapAlloc.deleteInstance(m_window);
		m_window = nullptr;
	}
}

//==============================================================================
Error App::create(const ConfigSet& config, 
	AllocAlignedCallback allocCb, void* allocCbUserData)
{
	Error err = createInternal(config, allocCb, allocCbUserData);
	if(err)
	{
		cleanup();
		ANKI_LOGE("App initialization failed");
	}

	return err;
}

//==============================================================================
Error App::createInternal(const ConfigSet& config_, 
	AllocAlignedCallback allocCb, void* allocCbUserData)
{
	m_allocCb = allocCb;
	m_allocCbData = allocCbUserData;
	m_heapAlloc = HeapAllocator<U8>(allocCb, allocCbUserData);
	ConfigSet config = config_;

	ANKI_CHECK(initDirs());

	// Print a message
	const char* buildType =
#if !ANKI_DEBUG
		"release";
#else
		"debug";
#endif

	ANKI_LOGI("Initializing application ("
		"version %u.%u, "
		"build %s %s, "
		"commit %s)...",
		ANKI_VERSION_MAJOR, ANKI_VERSION_MINOR,
		buildType,
		__DATE__,
		ANKI_REVISION);

	m_timerTick = 1.0 / 60.0; // in sec. 1.0 / period

#if ANKI_ENABLE_COUNTERS
	ANKI_CHECK(CountersManagerSingleton::get().create(
		m_heapAlloc, m_settingsDir.toCString(), &m_globalTimestamp));

	ANKI_CHECK(TraceManagerSingleton::get().create(
		m_heapAlloc, m_settingsDir.toCString()));
#endif

	//
	// Window
	//
	NativeWindow::Initializer nwinit;
	nwinit.m_width = config.get("width");
	nwinit.m_height = config.get("height");
	nwinit.m_majorVersion = config.get("glmajor");
	nwinit.m_minorVersion = config.get("glminor");
	nwinit.m_depthBits = 0;
	nwinit.m_stencilBits = 0;
	nwinit.m_fullscreenDesktopRez = config.get("fullscreenDesktopResolution");
	nwinit.m_debugContext = config.get("debugContext");
	m_window = m_heapAlloc.newInstance<NativeWindow>();
	if(!m_window) return ErrorCode::OUT_OF_MEMORY;

	ANKI_CHECK(m_window->create(nwinit, m_heapAlloc));

	m_ctx = m_window->getCurrentContext();
	m_window->contextMakeCurrent(nullptr);

	//
	// Input
	//
	m_input = m_heapAlloc.newInstance<Input>();
	if(!m_input) return ErrorCode::OUT_OF_MEMORY;

	ANKI_CHECK(m_input->create(m_window));

	//
	// Threadpool
	//
	m_threadpool = m_heapAlloc.newInstance<Threadpool>(getCpuCoresCount());
	if(!m_threadpool) return ErrorCode::OUT_OF_MEMORY;

	//
	// GL
	//
	m_gr = m_heapAlloc.newInstance<GrManager>();
	if(!m_gr)
	{
		return ErrorCode::OUT_OF_MEMORY;
	}

	GrManagerInitializer grInit;
	grInit.m_allocCallback = m_allocCb;
	grInit.m_allocCallbackUserData = m_allocCbData;
	grInit.m_makeCurrentCallback = makeCurrent;
	grInit.m_makeCurrentCallbackData = this;
	grInit.m_ctx = m_ctx;
	grInit.m_swapBuffersCallback = swapWindow;
	grInit.m_swapBuffersCallbackData = m_window;
	grInit.m_cacheDirectory = m_cacheDir.toCString();
	grInit.m_registerDebugMessages = nwinit.m_debugContext;

	ANKI_CHECK(m_gr->create(grInit));

	//
	// Physics
	//
	m_physics = m_heapAlloc.newInstance<PhysicsWorld>();
	if(!m_physics)
	{
		return ErrorCode::OUT_OF_MEMORY;
	}

	ANKI_CHECK(m_physics->create(m_allocCb, m_allocCbData));

	//
	// Resources
	//
	ResourceManager::Initializer rinit;
	rinit.m_gr = m_gr;
	rinit.m_physics = m_physics;
	rinit.m_config = &config;
	rinit.m_cacheDir = m_cacheDir.toCString();
	rinit.m_allocCallback = m_allocCb;
	rinit.m_allocCallbackData = m_allocCbData;
	rinit.m_tempAllocatorMemorySize = 1024 * 1024 * 4;
	m_resources = m_heapAlloc.newInstance<ResourceManager>();
	if(!m_resources) return ErrorCode::OUT_OF_MEMORY;

	ANKI_CHECK(m_resources->create(rinit));

	//
	// Renderer
	//
	if(nwinit.m_fullscreenDesktopRez)
	{
		config.set("width", m_window->getWidth());
		config.set("height", m_window->getHeight());
	}

	m_renderer = m_heapAlloc.newInstance<MainRenderer>();
	if(!m_renderer) return ErrorCode::OUT_OF_MEMORY;

	ANKI_CHECK(m_renderer->create(
		m_threadpool,
		m_resources,
		m_gr,
		m_heapAlloc,
		config,
		&m_globalTimestamp));

	ANKI_CHECK(m_resources->_setShadersPrependedSource(
		m_renderer->_getShadersPrependedSource().toCString()));

	// Scene
	m_scene = m_heapAlloc.newInstance<SceneGraph>();
	if(!m_scene) return ErrorCode::OUT_OF_MEMORY;

	ANKI_CHECK(m_scene->create(m_allocCb, m_allocCbData, 
		config.get("sceneFrameAllocatorSize"), m_threadpool, m_resources,
		m_input, &m_globalTimestamp));

	// Script
	m_script = m_heapAlloc.newInstance<ScriptManager>();
	if(!m_script) return ErrorCode::OUT_OF_MEMORY;
		
	ANKI_CHECK(m_script->create(m_allocCb, m_allocCbData, m_scene));

	ANKI_LOGI("Application initialized");
	return ErrorCode::NONE;
}

//==============================================================================
void App::makeCurrent(void* papp, void* ctx)
{
	ANKI_ASSERT(papp != nullptr);
	App* app = reinterpret_cast<App*>(papp);
	app->m_window->contextMakeCurrent(ctx);
}

//==============================================================================
void App::swapWindow(void* window)
{
	reinterpret_cast<NativeWindow*>(window)->swapBuffers();
}

//==============================================================================
Error App::initDirs()
{
#if ANKI_OS != ANKI_OS_ANDROID
	// Settings path
	String home;
	String::ScopeDestroyer homed(&home, m_heapAlloc);
	ANKI_CHECK(getHomeDirectory(m_heapAlloc, home));

	ANKI_CHECK(m_settingsDir.sprintf(m_heapAlloc, "%s/.anki", &home[0]));

	if(!directoryExists(m_settingsDir.toCString()))
	{
		ANKI_CHECK(createDirectory(m_settingsDir.toCString()));
	}

	// Cache
	ANKI_CHECK(m_cacheDir.sprintf(m_heapAlloc, "%s/cache", &m_settingsDir[0]));

	if(directoryExists(m_cacheDir.toCString()))
	{
		ANKI_CHECK(removeDirectory(m_cacheDir.toCString()));
	}

	ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
#else
	//ANKI_ASSERT(gAndroidApp);
	//ANativeActivity* activity = gAndroidApp->activity;

	// Settings path
	//settingsDir = String(activity->internalDataDir, alloc);
	settingsDir = String("/sdcard/.anki/");
	if(!directoryExists(settingsDir.c_str()))
	{
		createDirectory(settingsDir.c_str());
	}

	// Cache
	cacheDir = settingsDir + "/cache";
	if(directoryExists(cacheDir.c_str()))
	{
		removeDirectory(cacheDir.c_str());
	}

	createDirectory(cacheDir.c_str());
#endif

	return ErrorCode::NONE;
}

//==============================================================================
Error App::mainLoop(UserMainLoopCallback callback, void* userData)
{
	ANKI_LOGI("Entering main loop");
	Bool quit = false;

	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	ANKI_COUNTER_START_TIMER(FPS);
	while(!quit)
	{
		HighRezTimer timer;
		timer.start();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		// Update
		ANKI_CHECK(m_input->handleEvents());

		// User update
		ANKI_CHECK(callback(*this, userData, quit));

		ANKI_CHECK(m_scene->update(prevUpdateTime, crntTime, *m_renderer));

		ANKI_CHECK(m_renderer->render(*m_scene));

		m_gr->swapBuffers();
		ANKI_COUNTERS_RESOLVE_FRAME();

		// Sleep
		timer.stop();
		if(timer.getElapsedTime() < m_timerTick)
		{
			HighRezTimer::sleep(getTimerTick() - timer.getElapsedTime());
		}

		++m_globalTimestamp;
	}

	// Performance ends
	ANKI_COUNTER_STOP_TIMER_INC(FPS);
	ANKI_COUNTERS_FLUSH();
	ANKI_TRACE_FLUSH();

	return ErrorCode::NONE;
}

} // end namespace anki
