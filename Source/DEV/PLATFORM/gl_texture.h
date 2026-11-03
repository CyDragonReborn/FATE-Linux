#ifndef FATE_GL_TEXTURE_H
#define FATE_GL_TEXTURE_H

#include <GL/gl.h>

#include <string>

class GlTexture
{
public:
	GlTexture() = default;
	~GlTexture();

	GlTexture(const GlTexture&) = delete;
	GlTexture& operator=(const GlTexture&) = delete;

	bool LoadPng(const std::string& path);
	bool LoadJpeg(const std::string& path);
	bool LoadFromFile(const std::string& path);
	void Destroy();

	GLuint Id() const;
	int Width() const;
	int Height() const;

private:
	bool UploadPixels(int width, int height, const unsigned char* pixels);

	GLuint m_id = 0;
	int m_width = 0;
	int m_height = 0;
};

#endif