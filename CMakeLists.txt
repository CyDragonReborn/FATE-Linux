cmake_minimum_required(VERSION 3.20)

project(FATE_Linux LANGUAGES C CXX)

if(WIN32)
    add_subdirectory(fate_src/DEV)
else()
    include(CTest)
    add_subdirectory(native)
endif()
