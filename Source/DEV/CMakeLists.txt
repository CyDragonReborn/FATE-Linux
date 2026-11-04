cmake_minimum_required(VERSION 3.20)

project(FateLinuxPlatform LANGUAGES CXX)

find_package(SDL3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

option(FATE_ENABLE_STEAM "Enable Steamworks integration in the native game target" OFF)
if(NOT FATE_ENABLE_STEAM)
	add_compile_definitions(FATE_NO_STEAM)
endif()

set(FMOD_SDK_ROOT "" CACHE PATH "Path to the FMOD Engine SDK root")
if(NOT FMOD_SDK_ROOT AND DEFINED ENV{FMOD_SDK_ROOT})
	set(FMOD_SDK_ROOT "$ENV{FMOD_SDK_ROOT}")
endif()

set(FATE_BUILD_FMOD_COMPAT_DEFAULT OFF)
if(FMOD_SDK_ROOT)
	set(FATE_BUILD_FMOD_COMPAT_DEFAULT ON)
endif()
option(FATE_BUILD_FMOD_COMPAT "Build the FMOD Core compatibility adapter" ${FATE_BUILD_FMOD_COMPAT_DEFAULT})


if(FATE_BUILD_FMOD_COMPAT)
		if(NOT EXISTS "${FMOD_SDK_ROOT}/api/core/inc/fmod.hpp")
			message(FATAL_ERROR "Set FMOD_SDK_ROOT to the FMOD Linux SDK root")
		endif()

		find_library(FMOD_CORE_LIBRARY
			NAMES fmod
			PATHS "${FMOD_SDK_ROOT}/api/core/lib/x86_64"
			NO_DEFAULT_PATH
			REQUIRED
		)

		add_library(FMOD::Core SHARED IMPORTED GLOBAL)
		set_target_properties(FMOD::Core PROPERTIES
			IMPORTED_LOCATION "${FMOD_CORE_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${FMOD_SDK_ROOT}/api/core/inc"
		)
endif()

add_library(fate_platform STATIC
	PLATFORM/gl_renderer.cpp
	PLATFORM/gl_texture.cpp
	PLATFORM/sdl_platform.cpp
)

target_compile_features(fate_platform PUBLIC cxx_std_17)
target_include_directories(fate_platform PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/PLATFORM
)
target_link_libraries(fate_platform PUBLIC
	SDL3::SDL3
	OpenGL::GL
	JPEG::JPEG
	PNG::PNG
)

if(FATE_BUILD_FMOD_COMPAT)
	add_library(fate_audio SHARED
		PLATFORM/fmod_compat.cpp
	)

	target_compile_features(fate_audio PUBLIC cxx_std_17)
	target_include_directories(fate_audio PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/PLATFORM
	)
	target_link_libraries(fate_audio PUBLIC FMOD::Core)
endif()