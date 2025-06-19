#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
#include "../headers/chip8.h"

const int VIDEO_WIDTH = 64;
const int VIDEO_HEIGHT = 32;
const int WINDOW_SCALE = 10;
const int WINDOW_WIDTH = VIDEO_WIDTH * WINDOW_SCALE;
const int WINDOW_HEIGHT = VIDEO_HEIGHT * WINDOW_SCALE;

void handleInput(SDL_Event& event, Chip8& chip8) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_1: chip8.keypad[0x1] = 1; break;
            case SDLK_2: chip8.keypad[0x2] = 1; break;
            case SDLK_3: chip8.keypad[0x3] = 1; break;
            case SDLK_4: chip8.keypad[0xC] = 1; break;
            case SDLK_q: chip8.keypad[0x4] = 1; break;
            case SDLK_w: chip8.keypad[0x5] = 1; break;
            case SDLK_e: chip8.keypad[0x6] = 1; break;
            case SDLK_r: chip8.keypad[0xD] = 1; break;
            case SDLK_a: chip8.keypad[0x7] = 1; break;
            case SDLK_s: chip8.keypad[0x8] = 1; break;
            case SDLK_d: chip8.keypad[0x9] = 1; break;
            case SDLK_f: chip8.keypad[0xE] = 1; break;
            case SDLK_z: chip8.keypad[0xA] = 1; break;
            case SDLK_x: chip8.keypad[0x0] = 1; break;
            case SDLK_c: chip8.keypad[0xB] = 1; break;
            case SDLK_v: chip8.keypad[0xF] = 1; break;
        }
    } else if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
            case SDLK_1: chip8.keypad[0x1] = 0; break;
            case SDLK_2: chip8.keypad[0x2] = 0; break;
            case SDLK_3: chip8.keypad[0x3] = 0; break;
            case SDLK_4: chip8.keypad[0xC] = 0; break;
            case SDLK_q: chip8.keypad[0x4] = 0; break;
            case SDLK_w: chip8.keypad[0x5] = 0; break;
            case SDLK_e: chip8.keypad[0x6] = 0; break;
            case SDLK_r: chip8.keypad[0xD] = 0; break;
            case SDLK_a: chip8.keypad[0x7] = 0; break;
            case SDLK_s: chip8.keypad[0x8] = 0; break;
            case SDLK_d: chip8.keypad[0x9] = 0; break;
            case SDLK_f: chip8.keypad[0xE] = 0; break;
            case SDLK_z: chip8.keypad[0xA] = 0; break;
            case SDLK_x: chip8.keypad[0x0] = 0; break;
            case SDLK_c: chip8.keypad[0xB] = 0; break;
            case SDLK_v: chip8.keypad[0xF] = 0; break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM file>\n";
        return 1;
    }

    Chip8 chip8;

    if (!chip8.loadROM(argv[1])) {
        std::cerr << "Failed to load ROM: " << argv[1] << std::endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             VIDEO_WIDTH,
                                             VIDEO_HEIGHT);

    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    const int FPS = 60;
    const int frameDelay = 1000 / FPS;
    const int CYCLES_PER_FRAME = 10;

    while (running) {
        auto startTime = std::chrono::high_resolution_clock::now();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            handleInput(event, chip8);
        }

        for (int i = 0; i < CYCLES_PER_FRAME; ++i) {
            chip8.cycle();
        }

        SDL_UpdateTexture(texture, nullptr, chip8.display.data(), VIDEO_WIDTH * sizeof(uint32_t));

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        auto endTime = std::chrono::high_resolution_clock::now();
        uint32_t elapsedTime = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());

        if (frameDelay > elapsedTime) {
            SDL_Delay(frameDelay - elapsedTime);
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}