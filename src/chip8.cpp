#include "../headers/chip8.h"
#include <iostream>
#include <string_view>
#include <fstream>
#include <random>

bool Chip8::loadROM(std::string_view fileName) {
    std::ifstream file(fileName.data(), std::ios::binary | std::ios::ate);
    if (!file) return false;

    std::streamsize size = file.tellg(); 
    constexpr std::size_t loadAddress = 0x200;
    if (size > static_cast<std::streamsize>(memory.size() - loadAddress)) return false;   

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&memory[loadAddress]), size);

    pc = loadAddress;
    return true;
}

Chip8::Chip8() {
    pc = 0x200;
    I = sp = delayTimer = soundTimer = 0;
    memory.fill(0);
    V.fill(0);
    stack.fill(0);
    display.fill(0);
    keypad.fill(0);

    static constexpr std::array<uint8_t, 80> fontset = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80 
    };

    std::copy(fontset.begin(), fontset.end(), memory.begin() + 0x50);

    initializeOpcodeTables();
}

void Chip8::cycle() {
    opcode = (memory[pc] << 8 | memory[pc + 1]);
    pc += 2;

    const auto highNibble = (opcode & 0xF000) >> 12;
    if (OpFunc handler = mainTable[highNibble]; handler) {
        (this->*handler)();
    } else {
        std::cerr << std::hex << "Unknown opcode: 0x" << opcode << "\n";
    }

    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) --soundTimer;
}

void Chip8::initializeOpcodeTables() {
    mainTable[0x0] = &Chip8::table0Handler;
    mainTable[0x1] = &Chip8::op1NNN;
    mainTable[0x2] = &Chip8::op2NNN;
    mainTable[0x3] = &Chip8::op3XKK;
    mainTable[0x4] = &Chip8::op4XKK;
    mainTable[0x5] = &Chip8::op5XY0;
    mainTable[0x6] = &Chip8::op6XKK;
    mainTable[0x7] = &Chip8::op7XKK;
    mainTable[0x8] = &Chip8::table8Handler;
    mainTable[0x9] = &Chip8::op9XY0;
    mainTable[0xA] = &Chip8::opANNN;
    mainTable[0xB] = &Chip8::opBNNN;
    mainTable[0xC] = &Chip8::opCXKK;
    mainTable[0xD] = &Chip8::opDXYN;
    mainTable[0xE] = &Chip8::tableEHandler;
    mainTable[0xF] = &Chip8::tableFHandler;

    table0[0x0] = &Chip8::op00E0;
    table0[0xE] = &Chip8::op00EE;

    table8[0x0] = &Chip8::op8XY0;
    table8[0x1] = &Chip8::op8XY1;
    table8[0x2] = &Chip8::op8XY2;
    table8[0x3] = &Chip8::op8XY3;
    table8[0x4] = &Chip8::op8XY4;
    table8[0x5] = &Chip8::op8XY5;
    table8[0x6] = &Chip8::op8XY6;
    table8[0x7] = &Chip8::op8XY7;
    table8[0xE] = &Chip8::op8XYE;

    tableE[0xE] = &Chip8::opEX9E;
    tableE[0x1] = &Chip8::opEXA1;

    tableF[0x07] = &Chip8::opFX07;
    tableF[0x0A] = &Chip8::opFX0A;
    tableF[0x15] = &Chip8::opFX15;
    tableF[0x18] = &Chip8::opFX18;
    tableF[0x1E] = &Chip8::opFX1E;
    tableF[0x29] = &Chip8::opFX29;
    tableF[0x33] = &Chip8::opFX33;
    tableF[0x55] = &Chip8::opFX55;
    tableF[0x65] = &Chip8::opFX65;
}

void Chip8::table0Handler() {
    OpFunc handler = table0[opcode & 0x000F];
    if (handler) (this->*handler)();
}

void Chip8::table8Handler() {
    OpFunc handler = table8[opcode & 0x000F];
    if (handler) (this->*handler)();
}

void Chip8::tableEHandler() {
    OpFunc handler = tableE[opcode & 0x000F];
    if (handler) (this->*handler)();
}

void Chip8::tableFHandler() {
    OpFunc handler = tableF[opcode & 0x00FF];
    if (handler) (this->*handler)();
}

void Chip8::op00E0() {
    display.fill(0);
}

void Chip8::op00EE() {
    if (sp == 0) {
        std::cerr << "Stack underflow on RET (0x00EE)\n";
        return;
    }
    --sp;
    pc = stack[sp];
}

void Chip8::op1NNN() {
    pc = opcode & 0xFFF;
}

void Chip8::op2NNN() {
    if (sp >= stack.size()) {
        std::cerr << "Stack overflow on CALL (0x2NNN) at PC=0x" << std::hex << pc << "\n";
        return;
    }
    
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0xFFF;
}

void Chip8::op3XKK() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    if (V[x] == kk) pc += 2;
}

void Chip8::op4XKK() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0x00FF;
    
    if (V[x] != kk) pc += 2;
}

void Chip8::op5XY0() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4; 
    
    if (V[x] == V[y]) pc += 2;
}

void Chip8::op6XKK() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    V[x] = kk;
}

void Chip8::op7XKK() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    V[x] += kk;
}

void Chip8::op8XY0() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    V[x] = V[y];
}

void Chip8::op8XY1() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    
    V[x] |= V[y];
}

void Chip8::op8XY2() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    
    V[x] &= V[y];
}

void Chip8::op8XY3() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    
    V[x] ^= V[y];
}

void Chip8::op8XY4() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    
    uint16_t sum = V[x] + V[y];
    V[0xF] = (sum > 0xFF) ? 1 : 0;
    V[x] = static_cast<uint8_t>(sum);
}

void Chip8::op8XY5() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    V[0xF] = V[x] > V[y] ? 1 : 0;
    V[x] -= V[y];
}

void Chip8::op8XY6() {
    uint8_t x = (opcode & 0xF00) >> 8;
    
    V[0xF] = V[x] & 0x1;
    V[x] >>= 1; 
}

void Chip8::op8XY7() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    V[0xF] = V[y] > V[x] ? 1 : 0;
    V[x] = V[y] - V[x];
}

void Chip8::op8XYE() {
    uint8_t x = (opcode & 0xF00) >> 8;

    V[0xF] = (V[x] & 0x80) >> 7;
    V[x] <<= 1;
}

void Chip8::op9XY0() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if ((opcode & 0x000F) == 0) {
        if (V[x] != V[y]) pc += 2;
    }
}

void Chip8::opANNN() {
    I = opcode & 0x0FFF;
}

void Chip8::opBNNN() {
    pc = (opcode & 0x0FFF) + V[0x0];
}

void Chip8::opCXKK() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint8_t> dist(0, 255);

    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t kk = (opcode & 0x00FF);

    V[x] = dist(rng) & kk;
}

void Chip8::opDXYN() {
    uint8_t x = (opcode & 0xF00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;

    V[0xF] = 0;

    for (uint32_t row = 0; row < n; ++row) {
        uint8_t spriteByte = memory[I + row];

        for (uint32_t col = 0; col < 8; ++col) {
            uint8_t spritePixel = (spriteByte >> (7 - col)) & 0x1;

            uint32_t screenX = (V[x] + col) % 64;
            uint32_t screenY = (V[y] + row) % 32;
            uint32_t index = screenY * 64 + screenX;

            if (spritePixel) {
                if (display[index] == 0xFFFFFFFF) V[0xF] = 1;
                display[index] ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::opEX9E() {
    uint8_t x = (opcode & 0x0F00) >> 8;

    if (keypad[V[x]]) {
        pc += 2;
    }
}

void Chip8::opEXA1() {
    uint8_t x = (opcode & 0x0F00) >> 8;

    if (!keypad[V[x]]) {
        pc += 2;
    }
}

void Chip8::opFX07() {
   uint8_t x = (opcode & 0x0F00) >> 8;

   V[x] = delayTimer;
}

void Chip8::opFX0A() {
    uint8_t x = (opcode & 0x0F00) >> 8;

    bool keyPressed = false;
    for (uint8_t i = 0; i < 16; ++i) {
        if (keypad[i]) {
            V[x] = i;
            keyPressed = true;
            return;
        }
    }
    
    if (!keyPressed) {
        pc -= 2; 
    }
}

void Chip8::opFX15() {
    uint8_t x = (opcode & 0x0F00) >> 8;

    delayTimer = V[x];
}

void Chip8::opFX18() {
    uint8_t x = (opcode & 0x0F00) >> 8;

    soundTimer = V[x];
}

void Chip8::opFX1E() {
    uint8_t x = (opcode & 0x0F00) >> 8;

    I += V[x];
}

void Chip8::opFX29() {
    uint8_t x = (opcode & 0x0F00) >> 8;

    I = 0x50 + V[x] * 5;
}

void Chip8::opFX33() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t value = V[x];

    memory[I] = value / 100;
    memory[I+1] = (value / 10) % 10;
    memory[I+2] = value % 10;
}

void Chip8::opFX55() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    
    for (uint8_t i = 0; i <= x; ++i) {
        memory[I + i] = V[i];
    }
}

void Chip8::opFX65() {
    uint8_t x = (opcode & 0x0F00) >> 8;
    
    for (uint8_t i = 0; i <= x; ++i) {
        V[i] = memory[I + i];
    }
}