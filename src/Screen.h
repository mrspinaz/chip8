#pragma once

#include <cstdint>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

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
    Screen(std::vector<uint32_t>& d, std::vector<uint8_t>& in, uint8_t& sound);
    ~Screen();
    void draw_texture();
    void update_inputs();
    void update_audio();
    bool get_esc_state();
};
