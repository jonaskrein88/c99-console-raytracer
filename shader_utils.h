#pragma once

#include <stdint.h>
#include <math.h>
#include <float.h>
#include "vector.h"
#include "renderer.h"

// Helper macros for pure C min/max/clamp behaviors
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(val, min, max) (MIN(MAX((val), (min)), (max)))



static const float BAYERMATRIX[16] = {0,8,2,10,12,4,14,6,3,11,1,9,15,7,13,5};
static const float BAYER1x4[4] = {0,3,1,2};
static const float PI = 3.14159f;



float clamp01(float x);
float remap_to01(float value, float originalMin, float originalMax);
float remap_from01(float value, float min, float max);

float hash11(float p);
Vec2  hash22(Vec2 p);
Vec3  hash33(Vec3 p);

float luma(Color c) ;
float lerpf(float a, float b, float t);
Color rgb2hsv(Color in);
Color hsv2rgb(Color in);
Color dither(Color color, int x, int y, float amount);
float dither_float(float v, int x, int y, float amount, DitherPattern dither_pattern);






// maps an input brightness to a palette
// returns the two colors and their mixing ratio
float gradient_map(float value, const Color *colors, size_t num_colors, Color *out_color_a, Color *out_color_b);
Color gradient_map_interpolated(float value, const Color *colors, size_t num_colors);

float calculate_fresnel(Vec3 normal, Vec3 view_dir, float base_reflectivity);

Color sample_texture_bilinear(RenderSettings *renderSettings, size_t textureIndex, float u, float v);
Color sample_texture(RenderSettings *renderSettings, size_t textureIndex, float u, float v);
Color sample_matcap(RenderSettings *settings, size_t index, Normal N);
Color sample_environmnet(RenderSettings *renderSettings, Vec3 view, size_t texture);

uint8_t sample_stencil_buffer(RenderSettings *renderSettings, int16_t x, int16_t y);
AsciiPixel *sample_canvas(RenderSettings *renderSettings, int16_t x, int16_t y);




float calculate_fog(RenderSettings *settings, FragmentData *frag);



Color make_glitchy_dithering(Color in, size_t x, size_t y, float dither_amount);


Color blend(Color base, Color blend, BlendMode_Color mode);

void _replace_only_empty_char(AsciiPixel *out_pixel, const char* bytes);
void _ascii_shader_full(Color color, AsciiPixel *out_pixel);
void _ascii_shader_from_symbol_list(Color fg, Color bg, const char **symbols, size_t num_symbols, float luma, AsciiPixel *out_pixel);
void _ascii_shader_blocks(Color fg, Color bg, float luma, AsciiPixel *out_pixel);
void _ascii_shader_from_lut(Color color, AsciiLUT *lut, AsciiPixel *out_pixel);
