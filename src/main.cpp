#include "Chip8.h"
#include "Screen.h"

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

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
