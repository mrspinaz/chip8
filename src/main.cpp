#include "Chip8.h"
#include "Screen.h"

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


const int CPU_HZ = 600; // should be able to change this. CPU clock speed.
const int SCREEN_FPS = 60;
const int MSPF = 1000 / SCREEN_FPS; // milliseconds per frame. 60 FPS hardcoded.
const int CYCLES_PER_SCREEN_REFRESH = CPU_HZ / SCREEN_FPS;

const int STARTUP_TIME = 2000; // ms, time to display startup screen

void startup_seq(Chip8& chip, Screen& screen)
{   
    uint64_t total_time = 0;
    uint64_t start_time = SDL_GetTicks();
    while(total_time < STARTUP_TIME)
    {   
        uint64_t loop_st = SDL_GetTicks();

        for(int i = 0; i < CYCLES_PER_SCREEN_REFRESH; i++)
        {
            chip.CPU_cycle();
        }
        
        screen.draw_texture();
        chip.update_timers();
        screen.update_inputs();
        screen.update_audio();
        
        uint64_t elapsed = SDL_GetTicks() - loop_st;
        if(elapsed < MSPF)
        {
            SDL_Delay(MSPF - elapsed); // delay for the remaining time such that while loop executes once per ~16 ms
        } 
        total_time = SDL_GetTicks() - start_time;
    }
}

int main(int argc, char** args)
{

    Chip8 test_chip;
    Screen test_screen(test_chip.get_display(), test_chip.get_input(), test_chip.get_sound_timer()); // connect chip8 to screen
    
    test_chip.load_ROM("ROMs/IBM Logo.ch8");
    startup_seq(test_chip, test_screen);
    test_chip.reset();
    
    test_chip.load_ROM("ROMs/Astro Dodge [Revival Studios, 2008].ch8");
    while(!test_screen.get_esc_state())
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
        
        uint64_t elapsed = SDL_GetTicks() - loop_st;
        if(elapsed < MSPF)
        {
            SDL_Delay(MSPF - elapsed); // delay for the remaining time such that while loop executes once per ~16 ms
        } 
    }

	return 0;
}
