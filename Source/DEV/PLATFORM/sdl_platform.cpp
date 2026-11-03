#include "sdl_platform.h"

SdlPlatform::~SdlPlatform()
{
	Destroy();
}

bool SdlPlatform::Create(const char* title, int width, int height)
{
	if (m_window != nullptr || m_context != nullptr)
	{
		return false;
	}

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		return false;
	}
	m_sdlInitialized = true;

	if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2) ||
		!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1) ||
		!SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))
	{
		Destroy();
		return false;
	}

	m_window = SDL_CreateWindow(
		title,
		width,
		height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (m_window == nullptr)
	{
		Destroy();
		return false;
	}

	m_context = SDL_GL_CreateContext(m_window);
	if (m_context == nullptr || !SDL_GL_MakeCurrent(m_window, m_context))
	{
		Destroy();
		return false;
	}

	return true;
}

void SdlPlatform::Destroy()
{
	if (m_gamepad != nullptr)
	{
		SDL_CloseGamepad(m_gamepad);
		m_gamepad = nullptr;
	}
	if (m_context != nullptr)
	{
		SDL_GL_DestroyContext(m_context);
		m_context = nullptr;
	}
	if (m_window != nullptr)
	{
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
	if (m_sdlInitialized)
	{
		SDL_Quit();
		m_sdlInitialized = false;
	}
}

bool SdlPlatform::PollEvent(SDL_Event& event)
{
	if (!SDL_PollEvent(&event))
	{
		return false;
	}

	if (event.type == SDL_EVENT_GAMEPAD_ADDED && m_gamepad == nullptr)
	{
		m_gamepad = SDL_OpenGamepad(event.gdevice.which);
	}
	else if (event.type == SDL_EVENT_GAMEPAD_REMOVED &&
		m_gamepad != nullptr &&
		SDL_GetGamepadID(m_gamepad) == event.gdevice.which)
	{
		SDL_CloseGamepad(m_gamepad);
		m_gamepad = nullptr;
	}

	return true;
}

void SdlPlatform::SwapBuffers()
{
	if (m_window != nullptr)
	{
		SDL_GL_SwapWindow(m_window);
	}
}

SDL_Window* SdlPlatform::Window() const
{
	return m_window;
}

SDL_GLContext SdlPlatform::Context() const
{
	return m_context;
}

SDL_Gamepad* SdlPlatform::Gamepad() const
{
	return m_gamepad;
}

bool SdlPlatform::GamepadButtonDown(SDL_GamepadButton button) const
{
	return m_gamepad != nullptr && SDL_GetGamepadButton(m_gamepad, button);
}

float SdlPlatform::GamepadAxisValue(SDL_GamepadAxis axis) const
{
	if (m_gamepad == nullptr)
	{
		return 0.0f;
	}
	const Sint16 axisValue = SDL_GetGamepadAxis(m_gamepad, axis);
	return axisValue < 0
		? static_cast<float>(axisValue) / 32768.0f
		: static_cast<float>(axisValue) / 32767.0f;
}