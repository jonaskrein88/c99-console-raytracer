#pragma once
#define LUT_SIZE 32
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

#include "vector.h"
#include "renderer.h"



typedef struct AsciiLUT{
    AsciiPixel elements[LUT_SIZE][LUT_SIZE][LUT_SIZE];
} AsciiLUT;



typedef char ThreeBytes[3];

typedef struct AsciiPalette{
    ThreeBytes characters[32];
    float coverages[32];
    size_t length;
} AsciiPalette;


void ce_generate_ansi_16_palette(Color *colors_out);
void ce_generate_xterm_256_palette(Color *colors_out);

Color ce_get_closest_color(Color in, const Color *colors, size_t num_colors);

Color color_to_xterm_save(Color c);
Color color_new_xterm_save_color(float r, float g, float b);

AsciiPalette ascii_palette_from_string(char *s, float *coverages);

void ce_create_lookup_table(AsciiLUT *lut, const Color colors[], size_t num_colors, AsciiPalette ascii_palette, float gamma, float color_variance_penalty, float luma_variance_penalty);
AsciiPixel _lookup_lut(const AsciiLUT *lut, Color color);

bool ce_save_lut_binary(const AsciiLUT *lut, const char *filepath);
bool ce_load_lut_binary(AsciiLUT *lut, const char *filepath);