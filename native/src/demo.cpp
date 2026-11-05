#include "fate/renderer.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        bool self_test = false;
        std::string assets, image;
        for (int i = 1; i < argc; ++i) {
            const std::string arg(argv[i]);
            if (arg == "--self-test") self_test = true;
            else if (arg == "--assets" && i + 1 < argc) assets = argv[++i];
            else if (arg == "--image" && i + 1 < argc) image = argv[++i];
            else {
                std::cerr << "Usage: fate-platform-demo [--self-test] [--assets DIRECTORY --image RELATIVE_PATH]\n";
                return 2;
            }
        }
        if (assets.empty() != image.empty()) throw std::invalid_argument("Supply both --assets and --image");
        if (self_test && !image.empty()) throw std::invalid_argument("Self-test uses a built-in reference texture");
        fate::Runtime runtime;
        fate::Renderer renderer("FATE native platform demo (engine integration pending)", 800, 600, self_test);
        fate::Input input;
        fate::Gamepad gamepad;
        gamepad.discover();
        const std::uint8_t white[] = {self_test ? std::uint8_t(0) : std::uint8_t(255), 255, 255, 255};
        std::unique_ptr<fate::Texture> texture;
        if (!image.empty()) {
            fate::Assets files(assets);
            const auto decoded = fate::load_image(files.read(image));
            texture = std::make_unique<fate::Texture>(decoded.width, decoded.height, decoded.rgba.data());
        } else texture = std::make_unique<fate::Texture>(1, 1, white);
        // A real indexed, textured mesh. Left-handed projection places +Z in
        // front of the camera, as expected by the existing game engine.
        const fate::Vertex vertices[] = {
            {-1, -1, 2, 0, 0, -1, 255, 255, 255, 255, 0, 1},
            { 1, -1, 2, 0, 0, -1, 255, 255, 255, 255, 1, 1},
            { 1,  1, 2, 0, 0, -1, 255, 255, 255, 255, 1, 0},
            {-1,  1, 2, 0, 0, -1, 255, 255, 255, 255, 0, 0}
        };
        const std::uint16_t indices[] = {0, 2, 1, 0, 3, 2};
        while (!input.quit) {
            input.begin_frame();
            SDL_Event event;
            while (SDL_PollEvent(&event)) { input.handle(event); gamepad.handle(event); }
            if (input.pressed[SDL_SCANCODE_ESCAPE] || gamepad.button(SDL_GAMEPAD_BUTTON_BACK)) input.quit = true;
            if (input.quit) break;
            renderer.begin(0.08f, 0.12f, 0.18f);
            int width = 0, height = 0;
            SDL_GetWindowSizeInPixels(renderer.window(), &width, &height);
            if (width <= 0 || height <= 0) { SDL_Delay(16); continue; }
            renderer.transform(fate::identity(), fate::identity(),
                fate::perspective_lh(1.1f, static_cast<float>(width) / height, 0.1f, 100));
            renderer.draw(vertices, 4, indices, 6, texture.get());
            if (self_test) {
                const auto center = renderer.pixel(width / 2, height / 2);
                if (center[0] > 15 || center[1] < 240 || center[2] < 240)
                    throw std::runtime_error("Textured mesh did not reach the framebuffer");
                const auto corner = renderer.pixel(0, 0);
                if (corner[0] > 40 || corner[1] > 50 || corner[2] > 60)
                    throw std::runtime_error("Framebuffer clear or projection failed");
                std::cout << "Native SDL3/OpenGL textured mesh readback passed\n";
                return 0;
            }
            renderer.present();
            SDL_Delay(1);
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
