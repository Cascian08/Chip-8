#ifndef CHIP_8_H
#define CHIP_8_H

#include <cstdint>
#include <array>
#include <string_view>
#include <random>

class Chip8 {
public:
    Chip8();
    [[nodiscard]] bool loadROM(std::string_view fileName);
    void cycle();

    enum class OpcodeClass : uint8_t {
        SYS = 0x0, JMP = 0x1, CALL = 0x2, SE = 0x3, SNE = 0x4,
        SE_REG = 0x5, LD = 0x6, ADD = 0x7, ALU = 0x8,
        SNE_REG = 0x9, LD_I = 0xA, JMP_V0 = 0xB, RND = 0xC,
        DRW = 0xD, KEYP = 0xE, MISC = 0xF 
    };

    std::array<uint8_t, 16> keypad{};
    std::array<uint32_t, 64 * 32> display{};

private:
    using OpFunc = void (Chip8::*)();

    inline static constexpr std::size_t MEMORY_SIZE = 4096;
    inline static constexpr std::size_t REGISTER_SIZE = 16;
    inline static constexpr std::size_t STACK_SIZE = 16;
    inline static constexpr std::size_t FONTSET_START_ADDRESS = 0x50;
    inline static constexpr std::size_t ROM_START_ADDRESS = 0x200;
    inline static constexpr std::array<uint8_t, 80> fontset = {
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

    static std::mt19937 rng;
    static std::uniform_int_distribution<uint8_t> dist;

    std::array<uint8_t, MEMORY_SIZE> memory{};
    std::array<uint8_t, REGISTER_SIZE> V{};
    uint16_t I{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint16_t pc{};
    uint8_t sp{};
    std::array<uint16_t, STACK_SIZE> stack{};
    uint16_t opcode{};

    std::array<OpFunc, 0xF + 1> mainTable{};
    std::array<OpFunc, 0xE + 1> table0{};
    std::array<OpFunc, 0xE + 1> table8{};
    std::array<OpFunc, 0xE + 1> tableE{};
    std::array<OpFunc, 0x65 + 1> tableF{};

    void initializeOpcodeTables();
    void table0Handler();
    void table8Handler();
    void tableEHandler();
    void tableFHandler();

    void op00E0();
    void op00EE();
    void op1NNN();
    void op2NNN();
    void op3XKK();
    void op4XKK();
    void op5XY0();
    void op6XKK();
    void op7XKK();
    void op8XY0();
    void op8XY1();
    void op8XY2();
    void op8XY3();
    void op8XY4();
    void op8XY5();
    void op8XY6();
    void op8XY7();
    void op8XYE();
    void op9XY0();
    void opANNN();
    void opBNNN();
    void opCXKK();
    void opDXYN();
    void opEX9E();
    void opEXA1();
    void opFX07();
    void opFX0A();
    void opFX15();
    void opFX18();
    void opFX1E();
    void opFX29();
    void opFX33();
    void opFX55();
    void opFX65();
};

#endif