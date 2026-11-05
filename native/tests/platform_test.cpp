#include "fate/platform.hpp"
#include "fate/renderer.hpp"
#include "datafile.h"
#include "math3D.h"
#include "randomizer.h"
#include "textfile.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class F> void must_fail(F operation) {
    bool failed = false;
    try { operation(); } catch (const std::exception&) { failed = true; }
    require(failed, "Expected operation to reject invalid input");
}
struct TemporaryDirectory {
    std::filesystem::path path;
    TemporaryDirectory() {
        auto pattern = (std::filesystem::temp_directory_path() / "fate-native-test-XXXXXX").string();
        std::vector<char> storage(pattern.begin(), pattern.end());
        storage.push_back('\0');
        const auto* result = mkdtemp(storage.data());
        if (!result) throw std::runtime_error("Cannot create test directory");
        path = result;
    }
    ~TemporaryDirectory() { std::error_code error; std::filesystem::remove_all(path, error); }
};
}

int main() {
    try {
        fate::Runtime runtime;
        TemporaryDirectory directory;
        const auto root = directory.path / "assets";
        std::filesystem::create_directories(root / "MixedCase");
        const std::vector<std::uint8_t> bytes{0, 1, 127, 128, 255};
        fate::write_save(root / "MixedCase" / "Texture.BIN", bytes);
        fate::Assets assets(root);
        require(assets.read("mixedcase\\texture.bin") == bytes, "Windows asset path resolution failed");
        require(assets.read("MixedCase/Texture.BIN") == bytes, "Exact asset path failed");
        must_fail([&] { assets.read("../outside"); });
        must_fail([&] { assets.read("/etc/passwd"); });
        must_fail([&] { assets.read("C:\\FATE\\file"); });
        must_fail([&] { assets.read("not-present"); });
        fate::write_save(directory.path / "outside", bytes);
        std::filesystem::create_symlink(directory.path / "outside", root / "escape");
        must_fail([&] { assets.read("escape"); });
        fate::write_save(root / "MixedCase" / "Other", bytes);
        fate::write_save(root / "MixedCase" / "OTHER", bytes);
        must_fail([&] { assets.read("mixedcase/other"); });

        // Save replacement must preserve binary data and remove temporary files.
        fate::write_save(root / "MixedCase" / "Texture.BIN", {9, 0, 8});
        require(assets.read("mixedcase/texture.bin") == std::vector<std::uint8_t>({9, 0, 8}), "Save replacement failed");
        for (const auto& entry : std::filesystem::directory_iterator(root / "MixedCase"))
            require(entry.path().string().find(".tmp.") == std::string::npos, "Save temporary file leaked");
        std::filesystem::create_directory(root / "destination-is-directory");
        must_fail([&] { fate::write_save(root / "destination-is-directory", bytes); });
        require(std::filesystem::is_directory(root / "destination-is-directory"), "Failed save changed destination");
        for (const auto& entry : std::filesystem::directory_iterator(root))
            require(entry.path().string().find(".tmp.") == std::string::npos, "Failed save left temporary file");

        fate::Input input;
        SDL_Event event{};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.scancode = SDL_SCANCODE_A;
        input.handle(event);
        require(input.pressed[SDL_SCANCODE_A] && input.held[SDL_SCANCODE_A], "Key down failed");
        input.begin_frame();
        event.key.repeat = true;
        input.handle(event);
        require(!input.pressed[SDL_SCANCODE_A] && input.held[SDL_SCANCODE_A], "Key repeat created another press");
        event.type = SDL_EVENT_WINDOW_FOCUS_LOST;
        input.handle(event);
        require(!input.focused && !input.held[SDL_SCANCODE_A], "Focus loss left a stuck key");
        event.type = SDL_EVENT_MOUSE_WHEEL;
        event.wheel.y = 2;
        event.wheel.direction = SDL_MOUSEWHEEL_FLIPPED;
        input.handle(event);
        require(input.wheel == -2, "Wheel direction failed");
        input.begin_frame();
        require(input.wheel == 0, "Wheel was not reset");
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.text = "FATE";
        input.handle(event);
        require(input.text == "FATE", "Text input failed");
        event.type = SDL_EVENT_QUIT;
        input.handle(event);
        require(input.quit, "Quit event failed");

        fate::Gamepad pad;
        require(pad.axis(SDL_GAMEPAD_AXIS_LEFTX) == 0 && !pad.button(SDL_GAMEPAD_BUTTON_SOUTH), "Disconnected gamepad is not neutral");
        fate::Clock clock;
        SDL_Delay(2);
        require(clock.tick() > 0 && clock.seconds() > 0, "Monotonic clock did not advance");

        const auto projection = fate::perspective_lh(1, 1.5f, 1, 10);
        const auto depth = [&](float z) { return (z * projection[10] + projection[14]) / z; };
        require(std::abs(depth(1) + 1) < 0.00001f && std::abs(depth(10) - 1) < 0.00001f,
                "Left-handed projection has wrong near/far mapping");
        must_fail([&] { fate::perspective_lh(1, 1, 10, 1); });
        must_fail([&] { fate::load_image({1, 2, 3}); });
        // Uncompressed 1x1, 24-bit TGA with a red pixel (BGR on disk).
        const std::vector<std::uint8_t> tga{0,0,2,0,0,0,0,0,0,0,0,0,1,0,1,0,24,0,0,0,255};
        const auto decoded = fate::load_image(tga);
        require(decoded.width == 1 && decoded.height == 1 && decoded.rgba == std::vector<std::uint8_t>({255,0,0,255}),
                "Asset image decoding failed");

        // Exercise the original engine modules as 64-bit Linux code, including
        // npos handling when a token has no following delimiter or comment.
        static_assert(sizeof(::int32) == 4 && sizeof(::uint32) == 4);
        const auto text_path = root / "parser.dat";
        {
            std::ofstream file(text_path, std::ios::binary);
            file << "<NAME>:FATE\r\n<COUNT>:42 # comment\r\n";
        }
        CTextFile text;
        require(text.Open(text_path.string()), "Engine text file open failed");
        std::queue<std::string> tokens;
        text.TokenizeNextLine(&tokens, ":\r\n", '#');
        require(tokens.size() == 2 && tokens.front() == "<NAME>", "Engine tokenization failed");
        tokens.pop();
        require(tokens.front() == "FATE", "Engine npos handling truncated a token");
        text.Close();
        CDataFile data(text_path.string());
        auto* name = data.GetDataGroup("NAME");
        require(name && name->Items() == 1 && name->Item(0) == "FATE", "Original data-group parser failed");
        require(BoundsContains(D3DXVECTOR3(-2,-2,-2), D3DXVECTOR3(2,2,2),
                               D3DXVECTOR3(-1,-1,-1), D3DXVECTOR3(1,1,1)), "Original bounds math failed");
        const D3DXVECTOR3 x(1,0,0), y(0,1,0);
        D3DXVECTOR3 cross;
        D3DXVec3Cross(&cross, &x, &y);
        require(cross.z == 1, "Original vector math changed handedness");
        CRandomizer random;
        random.AddChoice(0, 0);
        random.AddChoice(1, 100);
        for (int i = 0; i < 100; ++i) require(random.GetRandom() == 1, "Original weighted randomizer failed");
        std::cout << "Native filesystem, saves, input, timer, projection and image tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
