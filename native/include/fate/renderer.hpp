#pragma once

#include "fate/platform.hpp"
#include <array>
#include <cstddef>

namespace fate {

// Matrices follow the game's D3D row-vector convention. Uploading row-major
// storage to OpenGL transposes it into the equivalent column-vector matrix.
using Matrix = std::array<float, 16>;
Matrix identity();
Matrix perspective_lh(float fov_y, float aspect, float near_plane, float far_plane);

struct Vertex {
    float x, y, z;
    float nx = 0, ny = 0, nz = -1;
    std::uint8_t r = 255, g = 255, b = 255, a = 255;
    float u = 0, v = 0;
};

class Texture {
public:
    Texture(int width, int height, const std::uint8_t* rgba);
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    unsigned id() const { return id_; }
private:
    unsigned id_ = 0;
};

// OpenGL objects must be destroyed before their owning Renderer/context.
class Renderer {
public:
    Renderer(const std::string& title, int width, int height, bool hidden = false);
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    void begin(float red, float green, float blue);
    void transform(const Matrix& world, const Matrix& view, const Matrix& projection);
    void draw(const Vertex* vertices, std::size_t vertex_count,
              const std::uint16_t* indices, std::size_t index_count,
              const Texture* texture = nullptr, bool alpha_blend = false);
    std::array<std::uint8_t, 4> pixel(int x, int y) const;
    void present();
    bool fullscreen(bool enabled);
    SDL_Window* window() const { return window_; }
private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;
};

struct Image {
    int width = 0, height = 0;
    std::vector<std::uint8_t> rgba;
};
Image load_image(const std::vector<std::uint8_t>& encoded);

} // namespace fate
