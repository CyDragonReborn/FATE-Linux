#ifndef FATE_SDL_PLATFORM_H
#define FATE_SDL_PLATFORM_H

#include <SDL3/SDL.h>

class SdlPlatform
{
public:
	SdlPlatform() = default;
	~SdlPlatform();

	SdlPlatform(const SdlPlatform&) = delete;
	SdlPlatform& operator=(const SdlPlatform&) = delete;

	bool Create(const char* title, int width, int height);
	void Destroy();

	bool PollEvent(SDL_Event& event);
	void SwapBuffers();

	SDL_Window* Window() const;
	SDL_GLContext Context() const;
	SDL_Gamepad* Gamepad() const;

private:
	SDL_Window* m_window = nullptr;
	SDL_GLContext m_context = nullptr;
	SDL_Gamepad* m_gamepad = nullptr;
	bool m_sdlInitialized = false;
};

#endif