#include "raylib.h"
#include "Seg16.h"
#include <vector>

Seg16::Seg16(){} //blank default constructor for initialization in Device.cpp

Seg16::Seg16(int texture_width, int texture_height, float scale_factor_in, Vector2 loc_in) : 
            char_rects(16), texture_cols(8), texture_rows(2), scale_factor(scale_factor_in), loc(loc_in)
{

    cutout_width = (float)texture_width / 8;
    cutout_height = (float)texture_height / 2;
    for(int i = 0; i < 16; i++)
    {
        float start_pos_x = cutout_width * (i % texture_cols);
        float start_pos_y = cutout_height * ( i / texture_cols);
        Rectangle rect{start_pos_x, start_pos_y, cutout_width, cutout_height};
        char_rects[i] = rect;
    }

    dest_rect = {loc.x, loc.y, cutout_width*scale_factor, cutout_height*scale_factor };
}

Vector2 Seg16::get_location()
{
    return loc;
}

Rectangle Seg16::disp_char_rect(int disp_val)
{
    return char_rects[disp_val];
}

Rectangle Seg16::get_dest_rect()
{
    return dest_rect;
}


