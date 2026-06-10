#include "Device.h"
#include "Seg16.h"
#include "raylib.h"
#include "DrawingUtility.h"
#include <stdexcept>
#include <iterator>
#include <iostream>
#include <string>
#include <sstream>

Device::Device(std::vector<uint8_t>& draw_buffer_in, std::vector<uint8_t>& inputs_in) 
                : draw_buffer(draw_buffer_in), inputs(inputs_in), intensity(64*32), reg_ptr(nullptr), disp_range_start(0), text_disp_range(4), tot_num_lines(256)
{
    // game window
    InitWindow(1920, 1080, "Game Window");
    SetTargetFPS(60);

    //device screen
    start_im = GenImageColor(64,32, BLACK);
    screen = LoadTextureFromImage(start_im);
    UnloadImage(start_im);
    source_rec = {0.0f,0.0f,64,32};
    dest_rec = {796,200, 64*6.8, 32*6.8};
    on_color = (Color){18,24,14, 160};
    off_color = (Color){0, 0, 0, 0};
    decay = 0.8;
    rise = 0.5;

    //memory display
    text_lines.resize(text_disp_range);

    //16-segment display
    seg_full_image = LoadImage("assets/16_seg_sprites.png");
    seg_full_texture = LoadTextureFromImage(seg_full_image);
    UnloadImage(seg_full_image);

    //background device
    background_image = LoadImage("assets/16 seg test.png");
    background_texture = LoadTextureFromImage(background_image);
    UnloadImage(background_image);

    //glass
    glass_image = LoadImage("assets/glass_and_frame.png");
    glass_texture = LoadTextureFromImage(glass_image);
    UnloadImage(glass_image);

    //setup assets

    // Set up 16 Segment cutout template
    Vector2 loc_a = {1218.0f,40.0f};
    Vector2 loc_b = {1253.0f, 40.0f};
    float scale_factor = 0.08;
    test_16a = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_a);
    test_16b = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_b);

}

Device::~Device()
{
    UnloadTexture(screen);
    UnloadTexture(background_texture);
    UnloadTexture(glass_texture);
    UnloadTexture(seg_full_texture);
    CloseWindow();
}

void _fetch_memory_lines(std::vector<uint8_t>* mem_ptr, std::vector<std::string>& text_lines, int disp_range_start, int text_disp_range)
{
    //fetch and format memory data for memory display. Directly modifies text_lines
    
    for(int i = 0; i < text_disp_range; i++)
    {
        std::stringstream line;
        int line_start_address = disp_range_start + (i*16);
        line << std::hex << line_start_address << "||"; 
        for(int offset = 0; offset < 16; offset++)
        {
            line << std::hex << (*mem_ptr)[disp_range_start + offset + i*16] << " ";
        }

        text_lines[i] =  line.str();
    }

}

void Device::update_screen()
{
    for (int px = 0; px < 64*32; px++) 
    {

        if (draw_buffer[px])
        {
            intensity[px] = intensity[px] + (1.0 - intensity[px])*rise;     
            screen_pixels[px] = (Color){on_color.r, on_color.g, on_color.b, (unsigned char)(on_color.a * intensity[px])};     
        }
        else
        {
            intensity[px] *= decay;       
            screen_pixels[px] = (Color){on_color.r, on_color.g, on_color.b, (unsigned char)(on_color.a * intensity[px])};   
        }
     
    }

    // retreive chip8 internals
    uint8_t num1 = ((*reg_ptr)[1] & 0xF0 ) >> 4;
    uint8_t num2 = (*reg_ptr)[1] & 0x0F;

    //build memory display text
    
    disp_range_start += (int)(GetMouseWheelMove())*16; // multiply by 16 because 16 bytes displayed per line.

    if(disp_range_start < 0)
    {
        disp_range_start = 0;
    }
    else if(disp_range_start > (tot_num_lines - text_disp_range))
    {
        disp_range_start = tot_num_lines - text_disp_range;
    }
    std::cout << disp_range_start << std::endl;
    _fetch_memory_lines(memory_ptr, text_lines, disp_range_start, text_disp_range);

    //update screen texture
    UpdateTexture(screen, screen_pixels);

    BeginDrawing();
        ClearBackground(WHITE);
       
        DrawTexture(background_texture, 0, 0, WHITE);

        BeginBlendMode(BLEND_ALPHA);
            //draw chip8 screen
            DrawTexturePro(screen, source_rec, dest_rec,(Vector2){0,0},0.0f, WHITE);

            //draw registers  
            DrawTexturePro(seg_full_texture, test_16a.disp_char_rect(num1), test_16a.get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
            DrawTexturePro(seg_full_texture, test_16b.disp_char_rect(num2), test_16b.get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
            
            DrawTexture(glass_texture, 0, 0, WHITE);
        EndBlendMode();

        //draw memory screen
        int ypos = 0;
        for(int i = disp_range_start; i < (disp_range_start + text_disp_range); i++)
        {
            ypos += 30;
            DrawText(text_lines[i].c_str(), 0, ypos, 30, BLACK);        
        }
        

    EndDrawing();
}
void Device::update_inputs()
{
    inputs[0] = IsKeyDown(KEY_X);
    inputs[1] = IsKeyDown(KEY_ONE);
    inputs[2] = IsKeyDown(KEY_TWO);
    inputs[3] = IsKeyDown(KEY_THREE);
    inputs[4] = IsKeyDown(KEY_Q);
    inputs[5] = IsKeyDown(KEY_W);
    inputs[6] = IsKeyDown(KEY_E);
    inputs[7] = IsKeyDown(KEY_A);
    inputs[8] = IsKeyDown(KEY_S);
    inputs[9] = IsKeyDown(KEY_D);
    inputs[10] = IsKeyDown(KEY_Z);
    inputs[11] = IsKeyDown(KEY_C);
    inputs[12] = IsKeyDown(KEY_FOUR);
    inputs[13] = IsKeyDown(KEY_R);
    inputs[14] = IsKeyDown(KEY_F);
    inputs[15] = IsKeyDown(KEY_V);
}
void Device::set_chip8_internals(std::vector<uint8_t>* reg_ptr_in, std::vector<uint8_t>* memory_in)
{
    reg_ptr = reg_ptr_in;
}




/*

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
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    SDL_RenderClear(renderer);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!mixer)
    {
        SDL_Log("Couldn't create mixer on default device: %s", SDL_GetError());
        throw std::runtime_error(SDL_GetError());
    }
    audio = MIX_CreateSineWaveAudio(mixer, 440, 0.25f, -1);   // -1: play forever. You can specify milliseconds otherwise to have a limit.
    audio2 = MIX_CreateSineWaveAudio(mixer, 2*440, 0.25f, -1);
    if (!audio || !audio2)
    {
        SDL_Log("Couldn't generate sinewave: %s", SDL_GetError());
        throw std::runtime_error(SDL_GetError());
    }
    track = MIX_CreateTrack(mixer);
    track2 = MIX_CreateTrack(mixer);
    if (!track || !track2)
    {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        throw std::runtime_error(SDL_GetError());
    }
    MIX_SetTrackAudio(track, audio);
    MIX_SetTrackAudio(track2, audio2);
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
        MIX_PlayTrack(track2, 0);
        sound_on = true;
    }
    else if(sound_timer == 0 && sound_on == true)
    {
        MIX_PauseTrack(track);
        MIX_PauseTrack(track2);
        sound_on = false;
    }
}

bool Screen::get_esc_state()
{
    // get state of esc key
    return keys[SDL_SCANCODE_ESCAPE];
}
    */
