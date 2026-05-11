#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

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
    uint8_t top = -1;
    public:
    Stack() : stk(16) {}
    void push(uint16_t val)
    {
        if (top >= 15)
        {
            std::cout << "stack overflow" << std::endl;
            return;
        }

        stk[++top] = val;
    }

    uint16_t pop()
    {
        if (top == 255)
        {
            std::cout << "stack underflow" << std::endl;
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
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    std::vector<uint32_t>& display;


    public:
    Screen(std::vector<uint32_t>& d) : display(d)
    {   
        SDL_Init( SDL_INIT_VIDEO );
        SDL_CreateWindowAndRenderer("chip8", 64*10, 32*10, 0, &window, &renderer);
        SDL_SetRenderScale(renderer, 10,10);
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
    }
    ~Screen()
    {
        SDL_Quit(); 
    }
    void draw_point()
    { 
        SDL_RenderPoint(renderer, 64/2, 32/2);   
    }
    void draw_texture()
    {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
        SDL_UpdateTexture( texture , NULL, display.data(), 64 * sizeof (uint32_t));
        SDL_RenderTexture(renderer, texture, NULL, NULL);
    }
    void present_render()
    {
        // updates screen
        SDL_RenderPresent(renderer);
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

    // for decoding operations
    void(Chip8::*)() OP; // pointer do the decoded operation function
    std::vector<void(Chip8::*)()> decoder_table;
    std::vector<void(Chip8::*)()> decoder_table0;

    public:
    Chip8() : memory(4096), reg(16), display(32 * 64, 0), input(16), decoder_table(0xF + 0x1), decoder_table0(0xE + 0x1), stack()
    {
        pc = PROGRAM_START_ADDRESS;
        std::copy(font.begin(), font.end(), memory.begin() + FONT_START_ADDRESS);

        // setup decoder table
        decoder_table[0x1] = &Chip8::OP_1NNN;
        decoder_table[0x6] = &Chip8::OP_6XNN;
        decoder_table[0x7] = &Chip8::OP_7XNN;
        decoder_table[0xA] = &Chip8::OP_ANNN;
        decoder_table[0xD] = &Chip8::OP_DXYN;

        decoder_table0[0x0] = &Chip8::OP_00E0;
        decoder_table0[0xE] = &Chip8::OP_00EE;

        
    }

    void load_ROM(const char* filename)
    {
        std::ifstream file(filename, std::ios::binary);

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
        pc +=2; 
    }
    // ======================= DECODING FUNCS =============================
    void decode()
    {
        // decode using function pointers
        // note we can technically execute in one line but I want to keep an explicit decode function.
        OP = decoder_table[opcode >> 12];
    }
    void decode_table0()
    {
        
        op = decoder_table0[opcode & 0x000F];
    }
    // =======================================================================
    void execute()
    {
        (this->*OP)(); // execute the appropriate function.
    }
    // =======================================================================
    // ============= OPCODE FUNCS ===================
    void OP_00E0()
    {
        // clear screen
        std::fill(display.begin(), display.end(), 0); 
    }
    void OP_00EE()
    {
        // set program counter to address at to of stack, the subtract 1 from stack pointer.
        pc = stack.pop();
    }
    void OP_1NNN()
    {
        // jump
        pc =  opcode & 0x0FFF;
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
    void OP_ANNN()
    {
        // set index register I to NNN
        indreg = opcode & 0x0FFF;
    }
    void OP_DXYN()
    {
        // draw N pixels tall sprite at mem location that indreg is holding to screen.
        // draw at horiz coord X in reg X and vert coord Y in reg Y.
        // sprite pixels flip display pixels. reg F set to 1 if any display pixels were turned off, otherwise 0;
        
        // fetch coords from regs X,Y
        uint8_t xind = (opcode >> 8) & 0x000F;
        uint8_t yind = (opcode >> 4) & 0x000F;
        uint8_t xcoord = reg[xind] & (0x40 -0x01); // mod 64
        uint8_t ycoord = reg[yind] & (0x20 - 0x01); // mod 32

        // sprite height
        uint8_t spr_h = opcode & 0x000F;

        // set reg F to 0
        reg[0x0F] = 0;

        // update pixels
        uint8_t pixel_byte;
        uint8_t pixel;
        for (uint8_t row = 0; row < spr_h; row++ )
        {   
            if((row + yind) >= 32)
            {
                // if you reach bottom of screen, stop
                break;
            }
            // sprite memory start location is stored in indreg
            pixel_byte = memory[indreg + row];
            for(uint8_t col = 0; col < 8; col++)
            {
                if((col + xind) >= 64 )
                {
                    // if you reach right edge of screen, stop drawing this row of pixels.
                    break;
                }
                // extract each pixel (bit) state from byte, from most to least significant.
                pixel = pixel_byte & (0b1000000 >> col); 
                if(pixel)
                {
                    if (display[64*(row + yind) + (col + xind)] == 0xFFFFFFFF)
                    {
                        reg[16] = 1;
                    }
                    display[64*(row + yind) + (col + xind)] = display[64*(row + yind) + (col + xind)] ^ 0xFFFFFFFF; // XOR 
                }
            }
        }
    }

    // DEBUGGING FUNCTIONS
    std::vector<uint32_t>& get_display()
    {
        return display;
    }
    void flip_pixel(uint16_t loc)
    {
        if (display[loc] == 0x00000000){
            display[loc] = 0xFFFFFDFF;
        }
        else{
            display[loc] = 0x00000000;
        }
    }


};

int main(int argc, char** args)
{

    Chip8 test_chip;
    Screen test_screen(test_chip.get_display());

    test_chip.flip_pixel(64);
    test_chip.flip_pixel(63);
    test_screen.draw_texture();
    test_screen.present_render();

    SDL_Delay(5000);

   
	return 0;
}
