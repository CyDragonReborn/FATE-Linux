#include "fate/renderer.hpp"
#include <SDL3/SDL_opengl.h>
#include <climits>
#include <cmath>
#include <limits>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "third_party/stb_image.h"

namespace fate {
Matrix identity() {
    return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}
Matrix perspective_lh(float fov_y, float aspect, float near_plane, float far_plane) {
    if (!(fov_y > 0 && fov_y < 3.14159265f && aspect > 0 && near_plane > 0 && far_plane > near_plane))
        throw std::invalid_argument("Invalid perspective projection");
    const float y = 1 / std::tan(fov_y / 2);
    // OpenGL's clip Z is [-W, W], while Direct3D's is [0, W].
    return {y / aspect, 0, 0, 0, 0, y, 0, 0,
            0, 0, (far_plane + near_plane) / (far_plane - near_plane), 1,
            0, 0, -2 * near_plane * far_plane / (far_plane - near_plane), 0};
}

Renderer::Renderer(const std::string& title, int width, int height, bool hidden) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    const SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
        (hidden ? SDL_WINDOW_HIDDEN : 0);
    window_ = SDL_CreateWindow(title.c_str(), width, height, flags);
    if (!window_) throw std::runtime_error(std::string("Create window: ") + SDL_GetError());
    context_ = SDL_GL_CreateContext(window_);
    if (!context_) {
        const std::string error = SDL_GetError();
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        throw std::runtime_error("Create OpenGL context: " + error);
    }
    SDL_GL_SetSwapInterval(hidden ? 0 : 1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // D3D's default clockwise front winding in its left-handed space.
    glFrontFace(GL_CW);
    glDisable(GL_CULL_FACE);
}
Renderer::~Renderer() {
    if (context_) SDL_GL_DestroyContext(context_);
    if (window_) SDL_DestroyWindow(window_);
}
void Renderer::begin(float red, float green, float blue) {
    int width, height;
    SDL_GetWindowSizeInPixels(window_, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(red, green, blue, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
void Renderer::transform(const Matrix& world, const Matrix& view, const Matrix& projection) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.data());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.data());
    glMultMatrixf(world.data());
}
void Renderer::draw(const Vertex* vertices, std::size_t vertex_count,
                    const std::uint16_t* indices, std::size_t index_count,
                    const Texture* texture, bool alpha_blend) {
    if (!vertices || !indices || vertex_count == 0 || index_count % 3 != 0 || index_count > INT_MAX)
        throw std::invalid_argument("Invalid indexed triangle mesh");
    for (std::size_t i = 0; i < index_count; ++i)
        if (indices[i] >= vertex_count) throw std::out_of_range("Mesh index exceeds vertex count");
    if (texture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture->id());
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    } else glDisable(GL_TEXTURE_2D);
    if (alpha_blend) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else glDisable(GL_BLEND);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].x);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vertices[0].r);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), &vertices[0].nx);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].u);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count), GL_UNSIGNED_SHORT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    const auto error = glGetError();
    if (error != GL_NO_ERROR) throw std::runtime_error("OpenGL draw error: " + std::to_string(error));
}
std::array<std::uint8_t, 4> Renderer::pixel(int x, int y) const {
    int width, height;
    SDL_GetWindowSizeInPixels(window_, &width, &height);
    if (x < 0 || y < 0 || x >= width || y >= height) throw std::out_of_range("Pixel outside framebuffer");
    std::array<std::uint8_t, 4> value{};
    glReadPixels(x, height - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, value.data());
    return value;
}
void Renderer::present() {
    if (!SDL_GL_SwapWindow(window_)) throw std::runtime_error(SDL_GetError());
}
bool Renderer::fullscreen(bool enabled) { return SDL_SetWindowFullscreen(window_, enabled); }

Texture::Texture(int width, int height, const std::uint8_t* rgba) {
    if (width <= 0 || height <= 0 || !rgba) throw std::invalid_argument("Invalid texture");
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    if (glGetError() != GL_NO_ERROR) {
        glDeleteTextures(1, &id_);
        id_ = 0;
        throw std::runtime_error("OpenGL texture upload failed");
    }
}
Texture::~Texture() { if (id_) glDeleteTextures(1, &id_); }

Image load_image(const std::vector<std::uint8_t>& encoded) {
    if (encoded.empty() || encoded.size() > INT_MAX) throw std::invalid_argument("Invalid encoded image size");
    Image image;
    auto* pixels = stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()),
                                        &image.width, &image.height, nullptr, 4);
    if (!pixels) throw std::runtime_error(std::string("Decode image: ") + stbi_failure_reason());
    try {
        image.rgba.assign(pixels, pixels + static_cast<std::size_t>(image.width) * image.height * 4);
    } catch (...) { stbi_image_free(pixels); throw; }
    stbi_image_free(pixels);
    return image;
}
} // namespace fate
