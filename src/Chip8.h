#pragma once

#include <cstdint>
#include <vector>
#include <random>

class Stack
{
    std::vector<uint16_t> stk;
    int8_t top = -1;
    public:
    Stack();
    void push(uint16_t val);
    uint16_t pop();
    uint8_t get_top();
};

class Chip8
{
    std::vector<uint8_t> memory; // RAM
    std::vector<uint8_t> reg; // reg named VX in some implementations
    std::vector<uint32_t> display; // display screen
    Stack stack; // to keep track of order of execution
    uint16_t indreg; // special 16-bit register for use in operations
    uint16_t pc; // program counter
    std::vector<uint8_t> input; // hex keypad
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t opcode;
    uint8_t draw_flag; //set true to redraw screen.

    // random number seed as current time
    std::mt19937 rng;
    std::uniform_int_distribution<uint8_t> dis;

    // for decoding operations
    void(Chip8::*OP)(); // pointer to the decoded operation function
    std::vector<void(Chip8::*)()> decoder_table; // vector of pointers to functions
    std::vector<void(Chip8::*)()> decoder_table0; // "                            "
    std::vector<void(Chip8::*)()> decoder_table8; // "                            "
    std::vector<void(Chip8::*)()> decoder_tableE; // "                            "
    std::vector<void(Chip8::*)()> decoder_tableF; // "                            "

    public:
    Chip8();

    void load_ROM(const char* filename);
    void fetch();
    void decode();
    void execute();
    void CPU_cycle();

    // ============= OPCODE FUNCS ===================
    void OP_NULL();
    void OP_00E0();
    void OP_00EE();
    void OP_1NNN();
    void OP_2NNN();
    void OP_3XNN();
    void OP_4XNN();
    void OP_5XY0();
    void OP_6XNN();
    void OP_7XNN();
    void OP_8XY0();
    void OP_8XY1();
    void OP_8XY2();
    void OP_8XY3();
    void OP_8XY4();
    void OP_8XY5();
    void OP_8XY6();
    void OP_8XY7();
    void OP_8XYE();
    void OP_9XY0();
    void OP_ANNN();
    void OP_BNNN();
    void OP_CXKK();
    void OP_DXYN();
    void OP_EX9E();
    void OP_EXA1();
    void OP_FX07();
    void OP_FX0A();
    void OP_FX15();
    void OP_FX18();
    void OP_FX1E();
    void OP_FX29();
    void OP_FX33();
    void OP_FX55();
    void OP_FX65();

    void update_timers();
    std::vector<uint32_t>& get_display();
    std::vector<uint8_t>& get_input();
    uint8_t& get_sound_timer();
};
