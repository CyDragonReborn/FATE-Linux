find_package(SDL3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_library(fate_native_platform
    src/platform.cpp
    src/assets.cpp
    src/renderer.cpp
)
target_compile_features(fate_native_platform PUBLIC cxx_std_17)
target_include_directories(fate_native_platform PUBLIC include)
target_include_directories(fate_native_platform PRIVATE ../fate_tools/shims)
target_link_libraries(fate_native_platform PUBLIC SDL3::SDL3 PRIVATE OpenGL::GL)
target_compile_options(fate_native_platform PRIVATE -Wall -Wextra -Wpedantic)

add_library(fate_native_core
    ../fate_src/DEV/UTILITIES/math3D.cpp
    ../fate_src/DEV/UTILITIES/textfile.cpp
    ../fate_src/DEV/UTILITIES/datafile.cpp
    ../fate_src/DEV/UTILITIES/randomizer.cpp
)
target_compile_features(fate_native_core PUBLIC cxx_std_17)
target_compile_definitions(fate_native_core PUBLIC FATE_NATIVE=1)
target_include_directories(fate_native_core PUBLIC
    include ../fate_src/DEV/UTILITIES ../fate_tools/shims)

option(FATE_BUILD_FMOD_COMPAT "Build the existing FSOUND API using native miniaudio" ON)
if(FATE_BUILD_FMOD_COMPAT)
    find_package(Threads REQUIRED)
    add_library(fate_native_audio STATIC
        ../fate_tools/shims/fmod_shim.cpp
        ../fate_tools/shims/shim_stb_vorbis.c
    )
    target_compile_features(fate_native_audio PRIVATE cxx_std_14)
    target_include_directories(fate_native_audio PUBLIC ../fate_tools/shims)
    target_link_libraries(fate_native_audio PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
endif()

# This exercises the platform backend; it is deliberately not named Fate.
# The original game still needs to be connected to this backend.
add_executable(fate-platform-demo src/demo.cpp)
target_link_libraries(fate-platform-demo PRIVATE fate_native_platform)
set_target_properties(fate-platform-demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(BUILD_TESTING)
    add_executable(fate-native-tests tests/platform_test.cpp)
    target_link_libraries(fate-native-tests PRIVATE fate_native_platform fate_native_core)
    add_test(NAME native-platform COMMAND fate-native-tests)
    set_tests_properties(native-platform PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy")
    add_test(NAME native-renderer COMMAND fate-platform-demo --self-test)
    set_tests_properties(native-renderer PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=1"
    )
endif()
