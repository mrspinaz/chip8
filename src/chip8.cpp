#include <cstdint>
#include <vector>
#include <iostream>

using namespace std;

const vector<uint8_t> font = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

const uint16_t PROGRAM_START_ADDRESS = 0x200;
const uint16_t FONT_START_ADDRESS = 0X50;

class Stack 
{
    vector<uint16_t> stk;
    uint8_t top = -1;
    public:
    Stack() : stk(16) {}
    void push(uint16_t val)
    {
        if (top >= 15)
        {
            cout << "stack overflow" << endl;
            return;
        }

        stk[++top] = val;
    }

    uint16_t pop()
    {
        if (top == 255 )
        {
            cout << "stack underflow" << endl;
            return 0;
        }
        return stk[top--];
    }

    uint8_t get_top()
    {
        return top;
    }

};

class Chip8 
{
    vector<uint8_t> memory; // 4 KiB memory
    vector<uint8_t> regs; // 16 8-bit registers
    vector<uint8_t> display;
    Stack stack; // 16 level stack for function execution address tracking
    uint16_t indreg; // stores mem addresses for operations
    uint16_t pc; // program counter
    vector<uint8_t> input;
    uint8_t delay_timer; 
    uint8_t sound_timer;

    public:
    Chip8() : memory(4096), regs(16), display(32 * 64), input(16), stack() 
    {
        pc = PROGRAM_START_ADDRESS;
        copy(font.begin(), font.end(), memory.begin() + FONT_START_ADDRESS);
    }

};

int main()
{

}