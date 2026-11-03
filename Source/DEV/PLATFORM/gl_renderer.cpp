#include "gl_renderer.h"

#include <GL/gl.h>

GlRenderer::GlRenderer(SdlPlatform& platform) : m_platform(platform)
{
}

void GlRenderer::SetViewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void GlRenderer::BeginFrame(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlRenderer::EndFrame()
{
	m_platform.SwapBuffers();
}

void GlRenderer::SetDepthTest(bool enabled)
{
	if (enabled)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

void GlRenderer::SetAlphaBlend(bool enabled)
{
	if (enabled)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}