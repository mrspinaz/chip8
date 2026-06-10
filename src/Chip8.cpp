#include "Chip8.h"
#include "font.h"

#include <iostream>
#include <fstream>
#include <ctime>

// ===================== Stack =====================

Stack::Stack() : stk(16) {}

void Stack::push(uint16_t val)
{
    if (top >= 15)
    {
        std::cout << "chip8 stack overflow" << std::endl;
        return;
    }

    stk[++top] = val;
}

uint16_t Stack::pop()
{
    if (top == 255)
    {
        std::cout << "chip8 stack underflow" << std::endl;
        return 0;
    }
    return stk[top--];
}

uint8_t Stack::get_top()
{
    return top;
}

void Stack::set_top(int8_t val)
{
    top = val;
}

// ===================== Chip8 =====================

Chip8::Chip8() : memory(4096), reg(16), display(64 * 32, 0), input(16),
          decoder_table(0xF + 0x1), decoder_table0(0xE + 0x1), decoder_table8(0xE + 0x1), decoder_tableE(0xE + 0x1), decoder_tableF(0x65 + 0x1),
          stack(),
          rng(std::time({})), dis(0, 255),
          sound_timer(0), delay_timer(0)
{
    pc = PROGRAM_START_ADDRESS;
    std::copy(font.begin(), font.end(), memory.begin() + FONT_START_ADDRESS);


    // setup decoder table
    std::fill(decoder_table.begin(), decoder_table.end(), &Chip8::OP_NULL);
    std::fill(decoder_table0.begin(), decoder_table0.end(), &Chip8::OP_NULL);
    std::fill(decoder_table8.begin(), decoder_table8.end(), &Chip8::OP_NULL);
    std::fill(decoder_tableE.begin(), decoder_tableE.end(), &Chip8::OP_NULL);
    std::fill(decoder_tableF.begin(), decoder_tableF.end(), &Chip8::OP_NULL);

    decoder_table[0x1] = &Chip8::OP_1NNN;
    decoder_table[0x2] = &Chip8::OP_2NNN;
    decoder_table[0x3] = &Chip8::OP_3XNN;
    decoder_table[0x4] = &Chip8::OP_4XNN;
    decoder_table[0x5] = &Chip8::OP_5XY0;
    decoder_table[0x6] = &Chip8::OP_6XNN;
    decoder_table[0x7] = &Chip8::OP_7XNN;
    decoder_table[0x9] = &Chip8::OP_9XY0;
    decoder_table[0xA] = &Chip8::OP_ANNN;
    decoder_table[0xB] = &Chip8::OP_BNNN;
    decoder_table[0xC] = &Chip8::OP_CXKK;
    decoder_table[0xD] = &Chip8::OP_DXYN;

    decoder_table0[0x00] = &Chip8::OP_00E0;
    decoder_table0[0x0E] = &Chip8::OP_00EE;

    decoder_table8[0x0] = &Chip8::OP_8XY0;
    decoder_table8[0x1] = &Chip8::OP_8XY1;
    decoder_table8[0x2] = &Chip8::OP_8XY2;
    decoder_table8[0x3] = &Chip8::OP_8XY3;
    decoder_table8[0x4] = &Chip8::OP_8XY4;
    decoder_table8[0x5] = &Chip8::OP_8XY5;
    decoder_table8[0x6] = &Chip8::OP_8XY6;
    decoder_table8[0x7] = &Chip8::OP_8XY7;
    decoder_table8[0xE] = &Chip8::OP_8XYE;

    decoder_tableE[0x1] = &Chip8::OP_EXA1;
    decoder_tableE[0xE] = &Chip8::OP_EX9E;

    decoder_tableF[0x07] = &Chip8::OP_FX07;
    decoder_tableF[0x0A] = &Chip8::OP_FX0A;
    decoder_tableF[0x15] = &Chip8::OP_FX15;
    decoder_tableF[0x18] = &Chip8::OP_FX18;
    decoder_tableF[0x1E] = &Chip8::OP_FX1E;
    decoder_tableF[0x29] = &Chip8::OP_FX29;
    decoder_tableF[0x33] = &Chip8::OP_FX33;
    decoder_tableF[0x55] = &Chip8::OP_FX55;
    decoder_tableF[0x65] = &Chip8::OP_FX65;


}
void Chip8::reset()
{
    // reset the chip to load a new ROM
    indreg = 0;
    pc = PROGRAM_START_ADDRESS;
    delay_timer = 0;
    sound_timer = 0;

    stack.set_top(-1);
    std::fill(reg.begin(), reg.end(), 0);
    std::fill(memory.begin(), memory.end(), 0);
    std::copy(font.begin(), font.end(), memory.begin() + FONT_START_ADDRESS); // reload sprites to memory

    //reset screen
    OP_00E0();

}
void Chip8::load_ROM(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if(!file)
    {
        std::cerr << "Failed to load ROM." << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> ROM_buffer(file_size);

    file.read((char*) &ROM_buffer[0], file_size);

    std::copy(ROM_buffer.begin(), ROM_buffer.end(), memory.begin() + PROGRAM_START_ADDRESS);
}

void Chip8::fetch()
{
    // fetch the opcode
    opcode = (memory[pc] << 8) | memory[pc+1]; // combine two memory bytes to 16-bit opcode.
}

void Chip8::decode()
{
    // decode using function pointers
    // note we can technically decode and execute in one line but I want to keep an explicit decode function.
    if(opcode >> 12 == 0x0)
    {
        // use table0
        OP = decoder_table0[opcode & 0x000F];
    }
    else if(opcode >> 12 == 0x8)
    {
        // use table8
        OP = decoder_table8[opcode & 0x000F];
    }
    else if(opcode >> 12 == 0xE)
    {
        // use table E
        OP = decoder_tableE[opcode & 0x000F];
    }
    else if(opcode >> 12 == 0xF)
    {
        // use tableF
        OP = decoder_tableF[opcode & 0x00FF];
    }
    else
    {
        // use table
        OP = decoder_table[opcode >> 12];
    }
}

void Chip8::execute()
{
    (this->*OP)(); // execute the appropriate function.
}

void Chip8::CPU_cycle()
{
    fetch();
    pc +=2; // increment by 2 as each instruction is 2 bytes
    decode();
    execute();
}

// ============= OPCODE FUNCS ===================
void Chip8::OP_NULL()
{}

void Chip8::OP_00E0()
{
    // clear screen
    std::fill(display.begin(), display.end(), 0);
}

void Chip8::OP_00EE()
{
    // set program counter to address at to of stack, then subtract 1 from stack pointer.
    // used to return from a subroutine, used in tandem with 2NNN
    pc = stack.pop();
}

void Chip8::OP_1NNN()
{
    // jump
    pc =  opcode & 0x0FFF;
}

void Chip8::OP_2NNN(){
    // call subroutine at memory location NNN
    stack.push(pc); // store current pc in stack so subroutine can return later
    pc = opcode & 0x0FFF;
}

void Chip8::OP_3XNN()
{
    // skip one instruction if val at VX is equal to NN
    uint8_t ind = (opcode >> 8) & 0x000F;
    uint8_t val = opcode & 0x00FF;
    if(reg[ind] == val)
    {
        pc +=2;
    }
}

void Chip8::OP_4XNN()
{
    // skip one instruction if val at VX is not equal to NN
    uint8_t ind = (opcode >> 8) & 0x000F;
    uint8_t val = opcode & 0x00FF;
    if(reg[ind] != val)
    {
        pc +=2;
    }
}

void Chip8::OP_5XY0()
{
    // skip one instruction if VX == VY
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    if(reg[indX] == reg[indY])
    {
        pc +=2;
    }
}

void Chip8::OP_6XNN()
{
    // set reg X to NN
    uint8_t ind = (opcode >> 8) & 0x000F;
    reg[ind] = opcode & 0x00FF;
}

void Chip8::OP_7XNN()
{
    // add value NN to reg X
    uint8_t ind = (opcode >> 8) & 0x000F;
    reg[ind] = reg[ind] + (opcode & 0x00FF);
}

void Chip8::OP_8XY0()
{
    // set VX to VY
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    reg[indX] = reg[indY];
}

void Chip8::OP_8XY1()
{
    // VX set to binary OR of VY
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    reg[indX] |= reg[indY];
}

void Chip8::OP_8XY2()
{
    // VX set to binary AND of VY
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    reg[indX] &= reg[indY];
}

void Chip8::OP_8XY3()
{
    // VX set to binary XOR of VY
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    reg[indX] ^= reg[indY];
}

void Chip8::OP_8XY4()
{
    // VX set to value VX+XY
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    uint16_t sum = reg[indX] + reg[indY];
    reg[indX] += reg[indY];
    if(sum > 0xFF)
    {
        reg[0xF] = 1;
    } 
    else
    {
        reg[0xF] = 0;
    }
}

void Chip8::OP_8XY5()
{
    // set VX to VX-VY
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    if(reg[indX] > reg[indY])
    {
        reg[0xF] = 1;
    }
    else
    {
        reg[0xF] = 0;
    }
    reg[indX] -= reg[indY];
}

void Chip8::OP_8XY6()
{
    // AMBIGUOUS INSTRUCTION
    // shift
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    uint8_t outshited_bit;

    reg[indX] = reg[indY]; // this step is skipped in later super-chip and chip-48. Make separate optional OPfunc without it
    outshited_bit = reg[indX] & 0x0001;
    reg[indX] = reg[indX] >> 1;
    if(outshited_bit)
    {
        reg[0xF] = 1;
    }
    else
    {
        reg[0xF] = 0;
    }
}

void Chip8::OP_8XY7()
{
    // set VX to VY-VX
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    if(reg[indY] > reg[indX])
    {
        reg[0xF] = 1;
    }
    else
    {
        reg[0xF] = 0;
    }
    reg[indX] = reg[indY] - reg[indX];
}

void Chip8::OP_8XYE()
{
    // AMBIGUOUS INSTRUCTION
    // shift
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    uint8_t outshited_bit;

    reg[indX] = reg[indY]; // this step is skipped in later super-chip and chip-48. Make separate optional OPfunc without it
    outshited_bit = (reg[indX] >> 7) & 0x0001;
    reg[indX] = reg[indX] << 1;
    if(outshited_bit)
    {
        reg[0xF] = 1;
    }
    else
    {
        reg[0xF] = 0;
    }
}

void Chip8::OP_9XY0()
{
    // skip one instruction if VX != VY
    uint8_t indX = (opcode >> 8) & 0x000F;
    uint8_t indY = (opcode >> 4) & 0x000F;
    if(reg[indX] != reg[indY])
    {
        pc +=2;
    }
}

void Chip8::OP_ANNN()
{
    // set index register I to NNN
    indreg = opcode & 0x0FFF;
}

void Chip8::OP_BNNN()
{
    pc = (opcode & 0x0FFF) + reg[0];
}

void Chip8::OP_CXKK()
{
    // rand num from 0-255 ANDed with kk. Result stored in VX
    uint8_t ind = (opcode >> 8) & 0x000F;
    uint8_t kk = opcode & 0x00FF;
    reg[ind] = dis(rng) & kk;
}

void Chip8::OP_DXYN()
{
    // draw N pixels tall sprite at mem location that indreg is holding to screen.
    // draw at horiz coord X in reg X and vert coord Y in reg Y.
    // sprite pixels flip display pixels (XOR). reg F set to 1 if any display pixels were turned off, otherwise 0;

    // fetch coords from regs X,Y
    uint8_t xind = (opcode >> 8) & 0x000F;
    uint8_t yind = (opcode >> 4) & 0x000F;
    uint8_t xcoord = reg[xind] & (0x40 -0x01); // mod 64
    uint8_t ycoord = reg[yind] & (0x20 - 0x01); // mod 32

    // sprite height
    uint8_t spr_h = opcode & 0x000F;

    // set reg F to 0
    reg[0xF] = 0;

    // update pixels
    uint8_t pixel_byte;
    uint8_t pixel;
    for (uint8_t row = 0; row < spr_h; row++ )
    {
        if((row + ycoord) >= 32)
        {
            // if you reach bottom of screen, stop
            break;
        }
        // sprite memory start location is stored in indreg
        pixel_byte = memory[indreg + row];
        for(uint8_t col = 0; col < 8; col++)
        {
            if((col + xcoord) >= 64 )
            {
                // if you reach right edge of screen, stop drawing this row of pixels.
                break;
            }
            // extract each pixel (bit) state from byte, from most to least significant.
            pixel = pixel_byte & (0b10000000 >> col);
            if(pixel)
            {
                uint8_t& px = display[64*(row + ycoord) + (col + xcoord)];
                if (px == 0x01)
                {
                    reg[0xF] = 1;
                    px = 0x00;
                }
                else
                {
                    px = 0x01;
                }
            }
        }
    }
}

void Chip8::OP_EX9E()
{
    // Skip next instruction if key with the value of Vx is pressed.
    uint8_t ind = (opcode >> 8) & 0x000F;
    if(input[reg[ind]] == 1)
    {
        pc += 2;
    }
}

void Chip8::OP_EXA1()
{
    // Skip next instruction if key with the value of Vx is NOT pressed.
    uint8_t ind = (opcode >> 8) & 0x000F;
    if(input[reg[ind]] == 0)
    {
        pc += 2;
    }
}

void Chip8::OP_FX07()
{
    // Set Vx = delay timer value.
    uint8_t ind = (opcode >> 8) & 0x000F;
    reg[ind] = delay_timer;
}

void Chip8::OP_FX0A()
{
    // Wait for a key press, store the value of the key in Vx.
    uint8_t ind = (opcode >> 8) & 0x000F;
    for(int i = 0 ; i < 16 ; i++)
    {
        if(input[i] == 1)
        {
            reg[ind] = i;
            return;
        }
    }
    // decrement pc to come back to this instruction, as pc increments each CPU cycle .
    pc -= 2;
}

void Chip8::OP_FX15()
{
    // DT is set equal to the value of Vx
    uint8_t ind = (opcode >> 8) & 0x000F;
    delay_timer = reg[ind];
}

void Chip8::OP_FX18()
{
    // ST is set equal to the value of Vx.
    uint8_t ind = (opcode >> 8) & 0x000F;
    sound_timer = reg[ind];
}

void Chip8::OP_FX1E()
{
    // The values of I and Vx are added, and the results are stored in I
    uint8_t ind = (opcode >> 8) & 0x000F;
    indreg = reg[ind] + indreg;
}

void Chip8::OP_FX29()
{
    // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
    uint8_t ind = (opcode >> 8) & 0x000F;
    indreg = FONT_START_ADDRESS + 5*reg[ind];
}

void Chip8::OP_FX33()
{
    // The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
    uint8_t ind = (opcode >> 8) & 0x000F;
    uint8_t val = reg[ind];
    memory[indreg] = val / 100; // integer div, decimal discarded
    memory[indreg+1] = (val / 10) % 10;
    memory[indreg+2] = val % 10;
}

void Chip8::OP_FX55()
{
    // Store registers V0 through Vx in memory starting at location I
    uint8_t end = (opcode >> 8) & 0x000F;
    for(int i = 0; i <= end ; i++)
    {
        memory[indreg+i] = reg[i];
    }
}
void Chip8::OP_FX65()
{
    // reads values from memory starting at location I into registers V0 through Vx
    uint8_t end = (opcode >> 8) & 0x000F;
    for(int i = 0; i <= end ; i++)
    {
        reg[i] = memory[indreg+i];
    }
}
void Chip8::update_timers()
{
    if(sound_timer > 0)
    {
        sound_timer--;
    }
    if(delay_timer > 0)
    {
        delay_timer--;
    }
}
std::vector<uint8_t>& Chip8::get_display()
{
    return display;
}

std::vector<uint8_t>& Chip8::get_input()
{
    return input;
}
uint8_t& Chip8::get_sound_timer()
{
    return sound_timer;
}
std::vector<uint8_t>* Chip8::get_reg_ptr()
{
    return &reg;
}
std::vector<uint8_t>* Chip8::get_mem_ptr()
{
    return &memory;
}
