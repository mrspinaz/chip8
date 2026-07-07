#include "Device.h"
#include "Seg16.h"
#include "raylib.h"
#include <stdexcept>
#include <iterator>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>


Device::Device(std::vector<uint8_t>& draw_buffer_in, std::vector<uint8_t>& inputs_in, int FPS) 
                : draw_buffer(draw_buffer_in), inputs(inputs_in), intensity(64*32), reg_ptr(nullptr), disp_range_start(0), tot_num_lines(256), 
                  seg16_a_vec(16), seg16_b_vec(16), seg16_indreg_vec(4), seg16_pc_vec(4)
{
    // game window
    InitWindow(1920, 1080, "Game Window");
    SetTargetFPS(FPS);

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
    mem_font_size = 15.5;
    text_disp_range = 11;
    mem_line_spacing = 15;
    mem_color = (Color){170, 170, 170, 150};
    mem_font = LoadFontEx("assets/FSEX302.ttf", mem_font_size, nullptr, 0);
    text_lines.resize(text_disp_range);
    mem_header = "OFFSET   HEX                        KYT-265 MONITOR v1.0";
    mem_divider = "--------------------------------------------------------";

    //disassembly display
    dis_font_size = 17;
    max_lines = 24;
    dis_line_spacing = 15;
    dis_font = LoadFontEx("assets/FSEX302.ttf", dis_font_size, nullptr, 0);
    dis_text_lines.resize(max_lines);
    head = 0;
    lines_filled = 0;
    dis_header = " ---- TRACE / DISASM ----";

    //16-segment display
    seg_full_image = LoadImage("assets/16_seg_atlas.png");
    seg_full_texture = LoadTextureFromImage(seg_full_image);
    UnloadImage(seg_full_image);

    //background device
    background_image = LoadImage("assets/background.png");
    background_texture = LoadTextureFromImage(background_image);
    UnloadImage(background_image);

    //glass
    glass_image = LoadImage("assets/glass_and_frame.png");
    glass_texture = LoadTextureFromImage(glass_image);
    UnloadImage(glass_image);

    //setup assets

    // Set up 16 Segment cutout templates
    float scale_factor = 0.055;
    int col_count = 0;
    float horiz_displacement = 0;
    float vert_displacement = 0;
    for(int i = 0; i < 16; i++)
    {
        Vector2 loc_0a = {1357.0f + horiz_displacement , 146.0f + vert_displacement};
        Vector2 loc_0b = {1357.0f+26.0f + horiz_displacement, 146.0f + vert_displacement};

        seg16_a_vec[i] = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_0a);
        seg16_b_vec[i] = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_0b);

        horiz_displacement += 107.5; 

        col_count += 1;
        if(col_count % 4 == 0)
        {
            vert_displacement += 71.5;
            horiz_displacement = 0;
        }
    }

    float indreg_horiz_displacement = 0.0;
    for(int j = 0; j < 4; j++)
    {
        Vector2 loc_indreg = {1629.0f + indreg_horiz_displacement, 70.0f};
        Vector2 loc_pc = {1476.0f + indreg_horiz_displacement, 70.0f};

        seg16_indreg_vec[j] = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_indreg);
        seg16_pc_vec[j] = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_pc);

        indreg_horiz_displacement += 25.5;
    }


    /*
    Vector2 loc_1a = {1465.0f, 146.0f};
    Vector2 loc_1b = {1465.0f + 26.0f, 146.0f};
    seg_1a = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_1a);
    seg_1b = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_1b);

    Vector2 loc_4a = {1357.0f,217.0f};
    Vector2 loc_4b = {1357.0f + 26.0f,217.0f};
    seg_4a = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_4a);
    seg_4b = Seg16(seg_full_texture.width, seg_full_texture.height, scale_factor, loc_4b);
    */


}

Device::~Device()
{
    UnloadTexture(screen);
    UnloadTexture(background_texture);
    UnloadTexture(glass_texture);
    UnloadTexture(seg_full_texture);
    UnloadFont(mem_font);
    CloseWindow();
}

void Device::_fetch_memory_lines(std::vector<uint8_t>* mem_ptr, std::vector<std::string>& text_lines, int disp_range_start, int text_disp_range)
{
    //fetch and format memory data for memory display. 
    
    int disp_range_start_ind = disp_range_start*16;
    for(int i = 0; i < text_disp_range; i++)
    {
        std::stringstream line;
        int line_start_address = disp_range_start_ind + (i*16);
        line << "0x" << std::uppercase << std::hex << std::setw(3) << std::setfill('0') << line_start_address << " || "; 
        for(int offset = 0; offset < 16; offset++)
        {
            line << std::hex << std::setw(2) << std::setfill('0') << (int)(*mem_ptr)[line_start_address + offset] << " ";
        }

        text_lines[i] =  line.str();
    }

}

void Device::_update_dissasembly_lines()
{
    /*
    Update head and insert new line into text container for disassembly display.
    The disassemly display is set up as a circular buffer.
    */

    std::stringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << *opcode_ptr;
    std::string divider = " -> ";
    dis_text_lines[head] = stream.str() + divider + (*instruction_ptr).str();

    head = (head + 1) % max_lines;

    lines_filled += 1;

    if(lines_filled > max_lines)
    {
        lines_filled = max_lines;
    }

}

void Device::update_screen()
{   
    //update game screen buffer
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


    //build memory display text
    disp_range_start -= (int)(GetMouseWheelMove()); // starting line number.

    if(disp_range_start < 0)
    {
        disp_range_start = 0;
    }
    else if(disp_range_start > (tot_num_lines - text_disp_range))
    {
        disp_range_start = tot_num_lines - text_disp_range;
    }

    _fetch_memory_lines(memory_ptr, text_lines, disp_range_start, text_disp_range);
    _update_dissasembly_lines();

    //update screen texture
    UpdateTexture(screen, screen_pixels);

    BeginDrawing();
        ClearBackground(WHITE);
       
        DrawTexture(background_texture, 0, 0, WHITE);

        BeginBlendMode(BLEND_ALPHA);
            //draw chip8 screen
            DrawTexturePro(screen, source_rec, dest_rec,(Vector2){0,0},0.0f, WHITE);

            //draw registers 
            for(int i = 0; i < 16; i++)
            {
                DrawTexturePro(seg_full_texture, seg16_a_vec[i].disp_char_rect(((*reg_ptr)[i] & 0xF0 ) >> 4), seg16_a_vec[i].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
                DrawTexturePro(seg_full_texture, seg16_b_vec[i].disp_char_rect((*reg_ptr)[i] & 0x0F), seg16_b_vec[i].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
            }

                DrawTexturePro(seg_full_texture, seg16_indreg_vec[0].disp_char_rect((*indreg_ptr & 0xF000 ) >> 12), seg16_indreg_vec[0].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
                DrawTexturePro(seg_full_texture, seg16_indreg_vec[1].disp_char_rect((*indreg_ptr & 0x0F00 ) >> 8), seg16_indreg_vec[1].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
                DrawTexturePro(seg_full_texture, seg16_indreg_vec[2].disp_char_rect((*indreg_ptr & 0x00F0 ) >> 4), seg16_indreg_vec[2].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
                DrawTexturePro(seg_full_texture, seg16_indreg_vec[3].disp_char_rect((*indreg_ptr & 0x000F )), seg16_indreg_vec[3].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);

                DrawTexturePro(seg_full_texture, seg16_pc_vec[0].disp_char_rect((*pc_ptr & 0xF000 ) >> 12), seg16_pc_vec[0].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
                DrawTexturePro(seg_full_texture, seg16_pc_vec[1].disp_char_rect((*pc_ptr & 0x0F00 ) >> 8), seg16_pc_vec[1].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
                DrawTexturePro(seg_full_texture, seg16_pc_vec[2].disp_char_rect((*pc_ptr & 0x00F0 ) >> 4), seg16_pc_vec[2].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
                DrawTexturePro(seg_full_texture, seg16_pc_vec[3].disp_char_rect((*pc_ptr & 0x000F )), seg16_pc_vec[3].get_dest_rect(), Vector2 {0.0f, 0.0f}, 0.0f, WHITE);

            //DrawTexture(glass_texture, 0, 0, WHITE);
        EndBlendMode();

        //draw memory screen
        float mem_ypos = 482;
        float mem_xpos = 937;
        DrawTextEx(mem_font, mem_header.c_str(), (Vector2){mem_xpos, mem_ypos-10}, mem_font_size, 1, mem_color);
         DrawTextEx(mem_font, mem_divider.c_str(), (Vector2){mem_xpos, mem_ypos}, mem_font_size, 1, mem_color);
        for(int i = 0; i < text_disp_range; i++)
        {
            mem_ypos += mem_line_spacing;
            DrawTextEx(mem_font, text_lines[i].c_str(), (Vector2){mem_xpos, mem_ypos}, mem_font_size, 1, mem_color);        
        }

        //draw disassembly lines
        float dis_ypos = 492;
        float dis_xpos = 1493;
        DrawTextEx(dis_font, dis_header.c_str(), (Vector2){dis_xpos, dis_ypos-20}, dis_font_size, 1, mem_color);
        int count = 0;
        int ind = (head - 1 + max_lines) % max_lines;
        while(count < lines_filled)
        {
            
            DrawTextEx(dis_font, dis_text_lines[ind].c_str(), (Vector2){dis_xpos+2, dis_ypos}, dis_font_size, 1, mem_color);
            count++;
            ind = (ind - 1 + max_lines) % max_lines;
            dis_ypos += dis_line_spacing;
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
void Device::set_chip8_internals(std::vector<uint8_t>* reg_ptr_in, std::vector<uint8_t>* memory_in, std::stringstream* instruction_in, uint16_t* opcode_in, uint16_t* indreg_in, uint16_t* pc_in)
{
    reg_ptr = reg_ptr_in;
    memory_ptr = memory_in;
    instruction_ptr = instruction_in;
    opcode_ptr = opcode_in;
    indreg_ptr = indreg_in;
    pc_ptr = pc_in;

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
