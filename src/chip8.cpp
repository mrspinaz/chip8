#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>

const std::vector<uint8_t> font = {
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
    std::vector<uint16_t> stk;
    int8_t top = -1; // top is a signed 8bit
    public:
    Stack() : stk(16) {}
    void push(uint16_t val)
    {
        if (top >= 15)
        {
            std::cout << "chip8 stack overflow" << std::endl;
            return;
        }

        stk[++top] = val;
    }

    uint16_t pop()
    {
        if (top < 0) // underflow check
        {
            std::cout << "chip8 stack underflow" << std::endl;
            return 0;
        }
        return stk[top--];
    }

    uint8_t get_top()
    {
        return top;
    }

};

class Screen
{
    // rendering
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    //audio
    MIX_Mixer *mixer = nullptr;
    MIX_Audio *audio = nullptr;
    MIX_Track *track = nullptr;

    std::vector<uint32_t>& display; // reference to pixel array from Chip8
    std::vector<uint8_t>& inputs;  // reference to input states memory from chip8
    uint8_t& sound_timer; // reference to sound timer from Chip8
    bool sound_on;

    //inputs
    SDL_Event event;
    const bool* keys;

    public:
    Screen(std::vector<uint32_t>& d, std::vector<uint8_t>& in, uint8_t& sound) : display(d), inputs(in), sound_timer(sound)
    {   
        if(!SDL_Init( SDL_INIT_VIDEO ))
        {
            SDL_Log("Couldn't initialize SDL.");
            throw std::runtime_error(SDL_GetError());
        }
        if(!MIX_Init())
        {
            SDL_Log("Couldn't initialize audio mixer.");
            throw std::runtime_error(SDL_GetError());
        }

        if(!SDL_CreateWindowAndRenderer("chip8", 64*14, 32*14, 0, &window, &renderer))
        {
            SDL_Log("Error creating window.");
            throw std::runtime_error(SDL_GetError());
        }

        SDL_SetRenderScale(renderer, 14, 14);
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

        mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
        if (!mixer)
        {
            SDL_Log("Couldn't create mixer on default device: %s", SDL_GetError());
            throw std::runtime_error(SDL_GetError());
        }
        audio = MIX_CreateSineWaveAudio(mixer, 300, 0.25f, -1);   // -1: play forever. You can specify milliseconds otherwise to have a limit.
        if (!audio) 
        {
            SDL_Log("Couldn't generate sinewave: %s", SDL_GetError());
            throw std::runtime_error(SDL_GetError());
        }
        track = MIX_CreateTrack(mixer);
        if (!track)
        {
            SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
            throw std::runtime_error(SDL_GetError());
        }
        MIX_SetTrackAudio(track, audio);
        sound_on = false;
        


        keys = SDL_GetKeyboardState(nullptr);
        
    }
    ~Screen()
    {
        MIX_Quit();
        SDL_Quit(); 
    }
    void draw_texture()
    {
        // takes pixel array from chip8 and renders to screan

        SDL_UpdateTexture( texture , NULL, display.data(), 64 * sizeof (uint32_t));
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    void update_inputs()
    {
        // update input keys 
        SDL_PumpEvents();
        inputs[0] = keys[SDL_SCANCODE_X];
        inputs[1] = keys[SDL_SCANCODE_1];
        inputs[2] = keys[SDL_SCANCODE_2];
        inputs[3] = keys[SDL_SCANCODE_3];
        inputs[4] = keys[SDL_SCANCODE_Q];
        inputs[5] = keys[SDL_SCANCODE_W];
        inputs[6] = keys[SDL_SCANCODE_E];
        inputs[7] = keys[SDL_SCANCODE_A];
        inputs[8] = keys[SDL_SCANCODE_S];
        inputs[9] = keys[SDL_SCANCODE_D];
        inputs[10] = keys[SDL_SCANCODE_Z];
        inputs[11] = keys[SDL_SCANCODE_C];
        inputs[12] = keys[SDL_SCANCODE_4];
        inputs[13] = keys[SDL_SCANCODE_R];
        inputs[14] = keys[SDL_SCANCODE_F];
        inputs[15] = keys[SDL_SCANCODE_V];
    }
    void update_audio()
    {
        if(sound_timer > 0 && sound_on == false){
            MIX_PlayTrack(track, 0);
            sound_on = true;
        }
        else if(sound_timer == 0 && sound_on == true)
        {
            MIX_PauseTrack(track);
            sound_on = false;
        }
        
    }
    bool get_esc_state()
    {
        // get state of esc key
        return keys[SDL_SCANCODE_ESCAPE];
    }
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
    Chip8() : memory(4096), reg(16), display(32 * 64, 0xFF000000), input(16),
              decoder_table(0xF + 0x1), decoder_table0(0xE + 0x1), decoder_table8(0xE + 0x1), decoder_tableE(0xE + 0x1), decoder_tableF(0x65 + 0x1), 
              stack(),  
              rng(std::time({})), dis(0, 255),
              sound_timer(0), delay_timer(0), draw_flag(0)
    {
        pc = PROGRAM_START_ADDRESS;
        std::copy(font.begin(), font.end(), memory.begin() + FONT_START_ADDRESS);

        // setup decoder table
        decoder_table[0x0] = &Chip8::OP_NULL;
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

    void load_ROM(const char* filename)
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
    void fetch()
    {
        // fetch the opcode
        opcode = (memory[pc] << 8) | memory[pc+1]; // combine two memory bytes to 16-bit opcode. 
        
    }
    void decode()
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
    void execute()
    {
        (this->*OP)(); // execute the appropriate function.
    }
    void CPU_cycle()
    {
        fetch();
        pc +=2; // increment by 2 as each instruction is 2 bytes
        decode();
        execute();
    }
    // ============= OPCODE FUNCS ===================
    void OP_NULL()
    {}
    void OP_00E0()
    {
        // clear screen
        std::fill(display.begin(), display.end(), 0xFF000000);
        draw_flag = 1;
    }
    void OP_00EE()
    {
        // set program counter to address at to of stack, then subtract 1 from stack pointer.
        // used to return from a subroutine, used in tandem with 2NNN
        pc = stack.pop();
    }
    void OP_1NNN()
    {
        // jump
        pc =  opcode & 0x0FFF;
    }
    void OP_2NNN(){
        // call subroutine at memory location NNN
        stack.push(pc); // store current pc in stack so subroutine can return later
        pc = opcode & 0x0FFF;
    }
    void OP_3XNN()
    {
        // skip one instruction if val at VX is equal to NN
        uint8_t ind = (opcode >> 8) & 0x000F;
        uint8_t val = opcode & 0x00FF;
        if(reg[ind] == val)
        {
            pc +=2;
        }
    }
    void OP_4XNN()
    {
        // skip one instruction if val at VX is not equal to NN
        uint8_t ind = (opcode >> 8) & 0x000F;
        uint8_t val = opcode & 0x00FF;
        if(reg[ind] != val)
        {
            pc +=2;
        }
    }
    void OP_5XY0()
    {
        // skip one instruction if VX == VY
        uint8_t indX = (opcode >> 8) & 0x000F;
        uint8_t indY = (opcode >> 4) & 0x000F;
        if(reg[indX] == reg[indY])
        {
            pc +=2;
        }
    }
    void OP_6XNN()
    {
        // set reg X to NN
        uint8_t ind = (opcode >> 8) & 0x000F;
        reg[ind] = opcode & 0x00FF;
    }
    void OP_7XNN()
    {
        // add value NN to reg X
        uint8_t ind = (opcode >> 8) & 0x000F;
        reg[ind] = reg[ind] + (opcode & 0x00FF);
    }
    void OP_8XY0()
    {
        // set VX to VY
        uint8_t indX = (opcode >> 8) & 0x000F;
        uint8_t indY = (opcode >> 4) & 0x000F;
        reg[indX] = reg[indY];
    }
    void OP_8XY1()
    {
        // VX set to binary OR of VY
        uint8_t indX = (opcode >> 8) & 0x000F;
        uint8_t indY = (opcode >> 4) & 0x000F;
        reg[indX] |= reg[indY];
    }
    void OP_8XY2()
    {
        // VX set to binary AND of VY
        uint8_t indX = (opcode >> 8) & 0x000F;
        uint8_t indY = (opcode >> 4) & 0x000F;
        reg[indX] &= reg[indY];
    }
    void OP_8XY3()
    {
        // VX set to binary XOR of VY
        uint8_t indX = (opcode >> 8) & 0x000F;
        uint8_t indY = (opcode >> 4) & 0x000F;
        reg[indX] ^= reg[indY];
    }
    void OP_8XY4()
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
    void OP_8XY5()
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
    void OP_8XY6()
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
    void OP_8XY7()
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
    void OP_8XYE()
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
    void OP_9XY0()
    {
        // skip one instruction if VX != VY
        uint8_t indX = (opcode >> 8) & 0x000F;
        uint8_t indY = (opcode >> 4) & 0x000F;
        if(reg[indX] != reg[indY])
        {
            pc +=2;
        }
    }
    void OP_ANNN()
    {
        // set index register I to NNN
        indreg = opcode & 0x0FFF;
    }
    void OP_BNNN()
    {
        pc = (opcode & 0x0FFF) + reg[0];
    }
    void OP_CXKK()
    {   
        // rand num from 0-255 ANDed with kk. Result stored in VX
        uint8_t ind = (opcode >> 8) & 0x000F;
        uint8_t kk = opcode & 0x00FF;
        reg[ind] = dis(rng) & kk;
    }
    void OP_DXYN()
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
                    uint32_t& px = display[64*(row + ycoord) + (col + xcoord)];
                    if (px == 0xFFFFFFFF)
                    {
                        reg[0xF] = 1;
                        px = 0xFF000000;
                    }
                    else
                    {
                        px = 0xFFFFFFFF;
                    }
                }
            }
        }
        draw_flag = 1;
    }
    void OP_EX9E()
    {
        // Skip next instruction if key with the value of Vx is pressed.
        uint8_t ind = (opcode >> 8) & 0x000F;
        if(input[reg[ind]] == 1)
        {
            pc += 2;
        }
    }
    void OP_EXA1()
    {
        // Skip next instruction if key with the value of Vx is NOT pressed.
        uint8_t ind = (opcode >> 8) & 0x000F;
        if(input[reg[ind]] == 0)
        {
            pc += 2;
        }
    }
    void OP_FX07()
    {
        // Set Vx = delay timer value.
        uint8_t ind = (opcode >> 8) & 0x000F;
        reg[ind] = delay_timer;
    }
    void OP_FX0A()
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
    void OP_FX15()
    {
        // DT is set equal to the value of Vx
        uint8_t ind = (opcode >> 8) & 0x000F;
        delay_timer = reg[ind];

    }
    void OP_FX18()
    {
        // ST is set equal to the value of Vx.
        uint8_t ind = (opcode >> 8) & 0x000F;
        sound_timer = reg[ind];
    }
    void OP_FX1E()
    {
        // The values of I and Vx are added, and the results are stored in I
        uint8_t ind = (opcode >> 8) & 0x000F;
        indreg = reg[ind] + indreg;
    }
    void OP_FX29()
    {
        // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
        uint8_t ind = (opcode >> 8) & 0x000F;
        indreg = FONT_START_ADDRESS + 5*reg[ind];
    }
    void OP_FX33()
    {
        // The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
        uint8_t ind = (opcode >> 8) & 0x000F;
        uint8_t val = reg[ind];
        memory[indreg] = val / 100; // integer div, decimal discarded 
        memory[indreg+1] = (val / 10) % 10;
        memory[indreg+2] = val % 10;

    }
    void OP_FX55()
    {
        // Store registers V0 through Vx in memory starting at location I
        uint8_t end = (opcode >> 8) & 0x000F;        
        for(int i = 0; i <= end ; i++)
        {
            memory[indreg+i] = reg[i];
        }
    }
    void OP_FX65()
    {
        // reads values from memory starting at location I into registers V0 through Vx
        uint8_t end = (opcode >> 8) & 0x000F;        
        for(int i = 0; i <= end ; i++)
        {
            reg[i] = memory[indreg+i];
        }
    }
    void update_timers()
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
    std::vector<uint32_t>& get_display()
    {
        return display;
    }
    std::vector<uint8_t>& get_input()
    {
        return input;
    }
    uint8_t& get_sound_timer()
    {
        return sound_timer;
    }
    uint8_t& get_draw_flag()
    {
        return draw_flag;
    }
    void set_draw_flag(uint8_t inp)
    {
        draw_flag = inp;
    }
};

int main(int argc, char** args)
{   

    const int CPU_HZ = 600; // should be able to change this. CPU clock speed.
    const int SCREEN_FPS = 60;
    const int MSPF = 1000 / SCREEN_FPS; // milliseconds per frame. 60 FPS hardcoded.
    const int CYCLES_PER_SCREEN_REFRESH = CPU_HZ / SCREEN_FPS;

    Chip8 test_chip;
    Screen test_screen(test_chip.get_display(), test_chip.get_input(), test_chip.get_sound_timer()); // connect chip8 to screen

    test_chip.load_ROM("ROMs/Astro Dodge [Revival Studios, 2008].ch8");
    std::cout << "testing" << std::endl;
    bool quit = false;
    while(!quit)
    {   
        uint64_t loop_st = SDL_GetTicks();

        for(int i = 0; i < CYCLES_PER_SCREEN_REFRESH; i++)
        {
            test_chip.CPU_cycle();
        }
        
        if(test_chip.get_draw_flag())
        {
            
            test_chip.set_draw_flag(0);
        }

        test_screen.draw_texture();
        test_chip.update_timers();
        test_screen.update_inputs();
        test_screen.update_audio();
        quit = test_screen.get_esc_state();

        uint64_t elapsed = SDL_GetTicks() - loop_st;
        if(elapsed < MSPF)
        {
            SDL_Delay(MSPF - elapsed); // delay for the remaining time such that while loop executes once per ~16 ms
        } 
    }

	return 0;
}
