#ifndef FATE_GL_RENDERER_H
#define FATE_GL_RENDERER_H

#include "sdl_platform.h"

class GlRenderer
{
public:
	explicit GlRenderer(SdlPlatform& platform);

	void SetViewport(int x, int y, int width, int height);
	void BeginFrame(float red, float green, float blue, float alpha);
	void EndFrame();
	void SetDepthTest(bool enabled);
	void SetAlphaBlend(bool enabled);
	void DrawTexturedQuad(unsigned int texture,
		float x, float y, float width, float height,
		float u0, float v0, float u1, float v1,
		float screenWidth, float screenHeight);

private:
	SdlPlatform& m_platform;
};

#endif