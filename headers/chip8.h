#ifndef CHIP_8_H
#define CHIP_8_H

#include <cstdint>
#include <array>
#include <string_view>

class Chip8 {
public:
    Chip8();
    [[nodiscard]] bool loadROM(std::string_view fileName);
    void cycle();

    std::array<uint8_t, 16> keypad{};
    std::array<uint32_t, 64 * 32> display{};

private:
    using OpFunc = void (Chip8::*)();

    inline static constexpr std::size_t MEMORY_SIZE = 4096;
    inline static constexpr std::size_t REGISTER_SIZE = 16;
    inline static constexpr std::size_t STACK_SIZE = 16;

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