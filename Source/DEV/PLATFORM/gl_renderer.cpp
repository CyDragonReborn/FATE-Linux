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

void GlRenderer::DrawTexturedQuad(unsigned int texture,
	float x, float y, float width, float height,
	float u0, float v0, float u1, float v1,
	float screenWidth, float screenHeight)
{
	GLint previousMatrixMode = GL_MODELVIEW;
	GLint previousTexture = 0;
	GLfloat previousColor[4];
	glGetIntegerv(GL_MATRIX_MODE, &previousMatrixMode);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
	glGetFloatv(GL_CURRENT_COLOR, previousColor);
	const GLboolean textureWasEnabled = glIsEnabled(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, screenWidth, screenHeight, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(u0, v0);
	glVertex2f(x, y);
	glTexCoord2f(u1, v0);
	glVertex2f(x + width, y);
	glTexCoord2f(u0, v1);
	glVertex2f(x, y + height);
	glTexCoord2f(u1, v1);
	glVertex2f(x + width, y + height);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glColor4fv(previousColor);
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
	if (textureWasEnabled == GL_FALSE)
	{
		glDisable(GL_TEXTURE_2D);
	}
	glMatrixMode(static_cast<GLenum>(previousMatrixMode));
}