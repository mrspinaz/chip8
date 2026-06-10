#pragma once
#include "raylib.h"
#include <vector>

class Seg16
{
    std::vector<Rectangle> char_rects;

    int texture_cols;
    int texture_rows;
    float cutout_width;
    float cutout_height;
    Vector2 loc;
    float scale_factor;
    Rectangle dest_rect; 

    public:
    Seg16(); // default constructor
    Seg16(int texture_width, int texture_height, float scale_factor, Vector2 loc);
    Rectangle disp_char_rect(int disp_val);
    Vector2 get_location();
    Rectangle get_dest_rect();

};