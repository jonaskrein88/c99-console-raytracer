#pragma once

#include "renderer.h"
#include "vector.h"





typedef struct ShaderParams_Post_MetaBalls{
    AsciiLUT *lut;
    size_t num_balls; 
    float dither_amount;
    float specular_amount;
    float ambient_amount;
    float diffuse_amount;
    Color *colors;
    size_t num_colors;
} ShaderParams_Post_MetaBalls;

void shader_post_metaballs(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);
Material ce_new_material_post_metaballs(AsciiLUT *lut, size_t num_balls, Color colors[], size_t num_colors, float ambient, float diffuse, float specular,float dither_amount);






typedef struct ShaderParams_Post_BlackHole{
    AsciiLUT *lut;
    float dither_amount;
    Color dark_color;
    Color bright_color;
    size_t tex_id;
} ShaderParams_Post_BlackHole;

void shader_post_black_hole(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);
Material ce_new_material_post_black_hole(AsciiLUT *lut, Color dark_color, Color bright_color, float dither_amount, size_t tex_id);