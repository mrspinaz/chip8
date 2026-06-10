#pragma once

#include "raylib.h"
#include "Seg16.h"
#include <cstdint>
#include <vector>
#include <string>


class Device
{
    //Chip8 Screen
    Image start_im;
    Texture2D screen;
    Rectangle source_rec;
    Rectangle dest_rec;
    Color screen_pixels[64*32];
    std::vector<float> intensity; //pixel intensities from 0.0-1.0
    Color on_color;
    Color off_color;
    float decay; // speed of pixel turn off for LCD
    float rise; //speed of pixel turn on for LCD
    std::vector<uint8_t>& draw_buffer;
    std::vector<uint8_t>& inputs; // key inputs

    //Memory Display 
    int disp_range_start; // value controlled by scroll wheel.
    int text_disp_range; // number of lines (16 bytes each) to display for memory readout.
    int tot_num_lines; // Hardcoded to 256, displaying 16 bytes per line, 4096 bytes total in memory.
    std::vector<std::string> text_lines;

    //16-Seg Display
    Image seg_full_image; // Image atlas
    Texture2D seg_full_texture; // Texture atlas
    float cutout_width_16seg;
    float cutout_height_16seg;
    Seg16 test_16a; // test 16-seg display 1
    Seg16 test_16b; // test 16-seg display 2

    //Rendered Device
    Texture2D background_texture;
    Image background_image;
    Image glass_image;
    Texture2D glass_texture;

    //chip8 internals pointers
    std::vector<uint8_t>* reg_ptr; // pointer to reg
    std::vector<uint8_t>* memory_ptr; // pointer to memory

    private:
    void _fetch_memory_lines(std::vector<uint8_t>* mem_ptr, std::vector<std::string>& text_lines, int disp_range_start, int text_disp_range);

    public:
    Device(std::vector<uint8_t>& draw_buffer_in, std::vector<uint8_t>& inputs_in);
    ~Device();
    void update_screen();
    void update_inputs();  
    void set_chip8_internals(std::vector<uint8_t>* reg_ptr_in, std::vector<uint8_t>* memory_in);
};

/*
class Screen
{
    // rendering
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    //audio
    MIX_Mixer *mixer = nullptr;
    MIX_Audio *audio = nullptr;
    MIX_Audio *audio2 = nullptr;
    MIX_Track *track = nullptr;
    MIX_Track *track2 = nullptr;

    std::vector<uint32_t>& display; // reference to pixel array from Chip8
    std::vector<uint8_t>& inputs;  // reference to input states memory from chip8
    uint8_t& sound_timer; // reference to sound timer from Chip8
    bool sound_on;

    //inputs
    SDL_Event event;
    const bool* keys;

    public:
    Screen(std::vector<uint32_t>& d, std::vector<uint8_t>& in, uint8_t& sound);
    ~Screen();
    void draw_texture();
    void update_inputs();
    void update_audio();
    bool get_esc_state();
};
*/