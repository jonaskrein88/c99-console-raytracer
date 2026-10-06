#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "vector.h"
#include "renderer.h"
#include "lookup.h"














typedef struct ShaderParams_Surface_GradientRamp{
    AsciiLUT *lut;
    float brightness;
    float ambient;
    float spec_amount;
    float spec_sharpness;
    DitherPattern dither_pattern;
    float dither_amount;
    uint8_t tex_id;
    Color *colors;
    uint8_t num_colors;
    const char **symbols;
    uint8_t num_symbols;
    bool discrete_steps;
    bool use_texture;
}ShaderParams_Surface_GradientRamp;
void shader_surface_gradient_ramp(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);


typedef struct ShaderParams_Surface_Tiles{
    float brightness;
    float ambient;
    float spec_amount;
    float spec_sharpness;
    DitherPattern dither_pattern;
    float dither_amount;
    bool use_texture;
    uint8_t tex_id;
    AsciiPixel *tiles;
    uint8_t num_tiles;
}ShaderParams_Surface_Tiles;
void shader_surface_tiles(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);



typedef struct {
    Color color;
    float ambient;
    float spec_amount;
    float spec_sharpness;
    float reflection_amount;
    float reflection_fresnel;
    float rim_light_amount;
    float dither_amount;
    bool fog_enabled;
    AsciiLUT *lut;
    bool use_texture;
    uint8_t tex_id;
} ShaderParams_Surface_Principled;
void shader_surface_principled(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);









typedef struct ShaderParams_Surface_Chrome{
    Color tint;
    uint8_t tex_id;
    float dither_amount;
    AsciiLUT *lut;
} ShaderParams_Surface_Chrome;
void shader_surface_chrome(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);




















typedef enum EnvironmentProjectionMethod{
    ENV_SPHERICAL,              // simple sphecial mapping with bilinear filtering
    ENV_SCREEN_FILL_BILINEAR,   // bilenear screen filling mapping
    ENV_SCREEN_FILL_POINT,      // screen filling mapping
    ENV_SCREEN_PIXEL_PERFECT    // unfiltered mapping based on pixel index. Will crop or tile on size mismatch
} EnvironmentProjectionMethod;

void shader_matcap(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);

typedef struct {
    uint8_t tex_id;
    float brightness;
    float dither_amount;
    EnvironmentProjectionMethod method;
    AsciiLUT *lut;
} ShaderParams_Environment_Texture;
void shader_environment_texture(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);




typedef struct {
    uint8_t opacity_tex_id;
    //float brightness;
    //float dither_amount;
    Color color;
    BlendMode_Color bg_blend_method;
    BlendMode_Color fg_blend_method;
    BlendMode_Ascii ascii_blend_method;
    //AsciiLUT *lut;
} ShaderParams_Sprite_Principled;
void shader_sprite_principled(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);






typedef struct {
    AsciiLUT *lut;
    float dither_amount;
} ShaderParams_Post_LUT;
void shader_post_lut(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);


typedef struct {
    AsciiLUT *lut;
    float dither_amount;
} ShaderParams_Post_Bloom;
void shader_post_bloom(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);

typedef struct {
    AsciiLUT *lut;
    uint8_t x_samples;
    uint8_t y_samples;
    float dither_amount;
} ShaderParams_Post_Blur;
void shader_post_blur(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);



typedef struct {
    AsciiLUT *lut;
    uint8_t num_samples;
    size_t size;
    float dither_amount;
    float dropout_chance;
} ShaderParams_Post_Dispersion;
void shader_post_dispersion(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);



typedef struct {
    AsciiLUT *lut;
    uint8_t stride;
    size_t num_samples;
    float dither_amount;
    float dropout_chance;
} ShaderParams_Post_Horizontal_Glitches;
void shader_post_horizontal_glitches(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);
void shader_post_horizontal_blur(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel);