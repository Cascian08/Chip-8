cmake_minimum_required(VERSION 3.15)
project(chip8_emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

find_package(SDL2 CONFIG REQUIRED)

add_executable(chip8 
    src/main.cpp 
    src/chip8.cpp
)

target_link_libraries(chip8 PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
)