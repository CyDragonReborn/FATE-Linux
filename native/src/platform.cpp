#include "fate/platform.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace fate {

Runtime::Runtime() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        throw std::runtime_error(std::string("SDL initialization: ") + SDL_GetError());
}
Runtime::~Runtime() { SDL_Quit(); }

void Input::begin_frame() {
    pressed.fill(false);
    mouse_pressed.fill(false);
    wheel = 0;
    text.clear();
}

void Input::handle(const SDL_Event& event) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        quit = true;
        break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        const auto key = event.key.scancode;
        if (key >= 0 && key < SDL_SCANCODE_COUNT) {
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (!held[key] && !event.key.repeat) pressed[key] = true;
                held[key] = true;
            } else held[key] = false;
        }
        break;
    }
    case SDL_EVENT_MOUSE_MOTION:
        mouse_x = event.motion.x;
        mouse_y = event.motion.y;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        const auto button = event.button.button;
        if (button < mouse_held.size()) {
            const bool down = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
            if (down && !mouse_held[button]) mouse_pressed[button] = true;
            mouse_held[button] = down;
        }
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL:
        wheel += event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -event.wheel.y : event.wheel.y;
        break;
    case SDL_EVENT_TEXT_INPUT:
        if (event.text.text) text += event.text.text;
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        focused = false;
        held.fill(false);
        pressed.fill(false);
        mouse_held.fill(false);
        mouse_pressed.fill(false);
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        focused = true;
        break;
    }
}

Gamepad::~Gamepad() { if (pad_) SDL_CloseGamepad(pad_); }
bool Gamepad::connected() const { return pad_ && SDL_GamepadConnected(pad_); }
void Gamepad::discover() {
    if (connected()) return;
    if (pad_) { SDL_CloseGamepad(pad_); pad_ = nullptr; }
    int count = 0;
    auto* ids = SDL_GetGamepads(&count);
    for (int i = 0; i < count && !pad_; ++i) pad_ = SDL_OpenGamepad(ids[i]);
    SDL_free(ids);
}
void Gamepad::handle(const SDL_Event& event) {
    if (event.type == SDL_EVENT_GAMEPAD_ADDED || event.type == SDL_EVENT_GAMEPAD_REMOVED)
        discover();
}
float Gamepad::axis(SDL_GamepadAxis axis, float deadzone) const {
    if (!connected()) return 0;
    deadzone = std::clamp(deadzone, 0.0f, 0.99f);
    const float value = std::clamp(SDL_GetGamepadAxis(pad_, axis) / 32767.0f, -1.0f, 1.0f);
    const float magnitude = std::abs(value);
    return magnitude <= deadzone ? 0 : std::copysign((magnitude - deadzone) / (1 - deadzone), value);
}
bool Gamepad::button(SDL_GamepadButton button) const {
    return connected() && SDL_GetGamepadButton(pad_, button);
}
bool Gamepad::rumble(float low, float high, uint32 milliseconds) {
    return connected() && SDL_RumbleGamepad(pad_,
        static_cast<Uint16>(std::clamp(low, 0.0f, 1.0f) * 65535),
        static_cast<Uint16>(std::clamp(high, 0.0f, 1.0f) * 65535), milliseconds);
}

Clock::Clock() : start_(SDL_GetTicksNS()), previous_(start_) {}
double Clock::tick() {
    const auto now = SDL_GetTicksNS();
    const double elapsed = (now - previous_) / 1e9;
    previous_ = now;
    return elapsed;
}
double Clock::seconds() const { return (SDL_GetTicksNS() - start_) / 1e9; }

std::filesystem::path user_data_directory() {
    char* path = SDL_GetPrefPath("FATE-Linux", "Fate");
    if (!path) throw std::runtime_error(std::string("Save directory: ") + SDL_GetError());
    std::filesystem::path result(path);
    SDL_free(path);
    return result;
}

} // namespace fate
