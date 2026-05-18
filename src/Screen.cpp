#include "Screen.h"

#include <stdexcept>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

Screen::Screen(std::vector<uint32_t>& d, std::vector<uint8_t>& in, uint8_t& sound)
    : display(d), inputs(in), sound_timer(sound)
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

Screen::~Screen()
{
    MIX_Quit();
    SDL_Quit();
}

void Screen::draw_texture()
{
    // takes pixel array from chip8 and renders to screan

    SDL_UpdateTexture( texture , NULL, display.data(), 64 * sizeof (uint32_t));
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void Screen::update_inputs()
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

void Screen::update_audio()
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

bool Screen::get_esc_state()
{
    // get state of esc key
    return keys[SDL_SCANCODE_ESCAPE];
}
