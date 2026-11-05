#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace fate {

// The save/asset formats use 32-bit integers. Linux's long is 64-bit.
using int32 = std::int32_t;
using uint32 = std::uint32_t;
static_assert(sizeof(int32) == 4 && sizeof(uint32) == 4);

struct Input {
    std::array<bool, SDL_SCANCODE_COUNT> held{};
    std::array<bool, SDL_SCANCODE_COUNT> pressed{};
    std::array<bool, 6> mouse_held{};
    std::array<bool, 6> mouse_pressed{};
    float mouse_x = 0, mouse_y = 0, wheel = 0;
    std::string text;
    bool quit = false;
    bool focused = true;

    void begin_frame();
    void handle(const SDL_Event& event);
};

class Runtime {
public:
    Runtime();
    ~Runtime();
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
};

class Gamepad {
public:
    Gamepad() = default;
    ~Gamepad();
    Gamepad(const Gamepad&) = delete;
    Gamepad& operator=(const Gamepad&) = delete;
    void handle(const SDL_Event& event);
    void discover();
    float axis(SDL_GamepadAxis axis, float deadzone = 0.15f) const;
    bool button(SDL_GamepadButton button) const;
    bool rumble(float low, float high, uint32 milliseconds);
    bool connected() const;
private:
    SDL_Gamepad* pad_ = nullptr;
};

class Clock {
public:
    Clock();
    double tick();
    double seconds() const;
private:
    std::uint64_t start_, previous_;
};

std::filesystem::path user_data_directory();

// Resolves Windows-style paths against a Linux asset tree without changing
// process CWD or allowing assets to escape the selected root.
class Assets {
public:
    explicit Assets(std::filesystem::path root);
    std::filesystem::path resolve(std::string name) const;
    std::vector<std::uint8_t> read(const std::string& name) const;
    const std::filesystem::path& root() const { return root_; }
private:
    std::filesystem::path root_;
};

// Atomic replacement in the same directory protects the previous save on
// failure; the caller supplies serialized game data.
void write_save(const std::filesystem::path& path, const std::vector<std::uint8_t>& data);

} // namespace fate
