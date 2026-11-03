#include "gl_texture.h"

#include <cstdio>
#include <cstdlib>
#include <csetjmp>
#include <cstddef>
#include <jpeglib.h>
#include <png.h>

#include <algorithm>
#include <cctype>
#include <limits>
#include <vector>

namespace
{
struct JpegErrorManager
{
	jpeg_error_mgr base;
	std::jmp_buf jumpBuffer;
};

struct JpegDecodeContext
{
	FILE* file;
	unsigned char* pixels;
	unsigned char* row;
	jpeg_decompress_struct decoder;
	JpegErrorManager error;
	bool decoderCreated;
};

void JpegErrorExit(j_common_ptr decoder)
{
	JpegErrorManager* error = reinterpret_cast<JpegErrorManager*>(decoder->err);
	std::longjmp(error->jumpBuffer, 1);
}

bool DecodeJpeg(const std::string& path, unsigned char** pixels, int* width, int* height)
{
	JpegDecodeContext* context = static_cast<JpegDecodeContext*>(std::calloc(1, sizeof(JpegDecodeContext)));
	if (context == nullptr)
	{
		return false;
	}

	if (setjmp(context->error.jumpBuffer) != 0)
	{
		if (context->decoderCreated)
		{
			jpeg_destroy_decompress(&context->decoder);
		}
		if (context->file != nullptr)
		{
			std::fclose(context->file);
		}
		std::free(context->row);
		std::free(context->pixels);
		std::free(context);
		return false;
	}

	context->file = std::fopen(path.c_str(), "rb");
	if (context->file == nullptr)
	{
		std::free(context);
		return false;
	}

	context->decoder.err = jpeg_std_error(&context->error.base);
	context->error.base.error_exit = JpegErrorExit;
	context->decoderCreated = true;
	jpeg_create_decompress(&context->decoder);
	jpeg_stdio_src(&context->decoder, context->file);
	jpeg_read_header(&context->decoder, TRUE);
	context->decoder.out_color_space = JCS_RGB;
	jpeg_start_decompress(&context->decoder);

	const size_t imageWidth = context->decoder.output_width;
	const size_t imageHeight = context->decoder.output_height;
	if (imageWidth == 0 || imageHeight == 0 ||
		imageWidth > static_cast<size_t>(std::numeric_limits<int>::max()) ||
		imageHeight > static_cast<size_t>(std::numeric_limits<int>::max()) ||
		imageWidth > std::numeric_limits<size_t>::max() / 4 / imageHeight ||
		context->decoder.output_components != 3)
	{
		jpeg_destroy_decompress(&context->decoder);
		std::fclose(context->file);
		std::free(context);
		return false;
	}

	context->pixels = static_cast<unsigned char*>(std::malloc(imageWidth * imageHeight * 4));
	context->row = static_cast<unsigned char*>(std::malloc(imageWidth * 3));
	if (context->pixels == nullptr || context->row == nullptr)
	{
		jpeg_destroy_decompress(&context->decoder);
		std::fclose(context->file);
		std::free(context->row);
		std::free(context->pixels);
		std::free(context);
		return false;
	}

	while (context->decoder.output_scanline < context->decoder.output_height)
	{
		const size_t rowIndex = context->decoder.output_scanline;
		JSAMPROW rowPointer = context->row;
		jpeg_read_scanlines(&context->decoder, &rowPointer, 1);
		unsigned char* output = context->pixels + rowIndex * imageWidth * 4;
		for (size_t column = 0; column < imageWidth; ++column)
		{
			output[column * 4 + 0] = context->row[column * 3 + 0];
			output[column * 4 + 1] = context->row[column * 3 + 1];
			output[column * 4 + 2] = context->row[column * 3 + 2];
			output[column * 4 + 3] = 255;
		}
	}

	*width = static_cast<int>(imageWidth);
	*height = static_cast<int>(imageHeight);
	* pixels = context->pixels;
	context->pixels = nullptr;
	jpeg_finish_decompress(&context->decoder);
	jpeg_destroy_decompress(&context->decoder);
	std::fclose(context->file);
	std::free(context->row);
	std::free(context);
	return true;
}
}

GlTexture::~GlTexture()
{
	Destroy();
}

bool GlTexture::LoadPng(const std::string& path)
{
	png_image image{};
	image.version = PNG_IMAGE_VERSION;
	if (!png_image_begin_read_from_file(&image, path.c_str()))
	{
		return false;
	}

	if (image.width == 0 || image.height == 0 ||
		image.width > static_cast<png_uint_32>(std::numeric_limits<int>::max()) ||
		image.height > static_cast<png_uint_32>(std::numeric_limits<int>::max()))
	{
		png_image_free(&image);
		return false;
	}

	image.format = PNG_FORMAT_RGBA;
	const int width = static_cast<int>(image.width);
	const int height = static_cast<int>(image.height);
	std::vector<png_byte> pixels(PNG_IMAGE_SIZE(image));
	if (!png_image_finish_read(&image, nullptr, pixels.data(), 0, nullptr))
	{
		png_image_free(&image);
		return false;
	}

	const bool uploaded = UploadPixels(width, height, pixels.data());
	png_image_free(&image);
	return uploaded;
}


bool GlTexture::LoadJpeg(const std::string& path)
	{
	unsigned char* pixels = nullptr;
	int width = 0;
	int height = 0;
	if (!DecodeJpeg(path, &pixels, &width, &height))
	{
		return false;
	}

	const bool uploaded = UploadPixels(width, height, pixels);
	std::free(pixels);
	return uploaded;
}

bool GlTexture::LoadFromFile(const std::string& path)
{
	const size_t extensionPosition = path.find_last_of('.');
	if (extensionPosition == std::string::npos)
	{
		return false;
	}

	std::string extension = path.substr(extensionPosition);
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char character)
	{
		return static_cast<char>(std::tolower(character));
	});

	if (extension == ".png")
	{
		return LoadPng(path);
	}
	if (extension == ".jpg" || extension == ".jpeg")
	{
		return LoadJpeg(path);
	}
	return false;
}

bool GlTexture::UploadPixels(int width, int height, const unsigned char* pixels)
{
	GLuint texture = 0;
	GLint previousTexture = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));

	if (texture == 0)
	{
		return false;
	}

	Destroy();
	m_id = texture;
	m_width = width;
	m_height = height;
	return true;
}

void GlTexture::Destroy()
{
	if (m_id != 0)
	{
		glDeleteTextures(1, &m_id);
		m_id = 0;
	}
	m_width = 0;
	m_height = 0;
}

GLuint GlTexture::Id() const
{
	return m_id;
}

int GlTexture::Width() const
{
	return m_width;
}

int GlTexture::Height() const
{
	return m_height;
}