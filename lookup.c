#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <float.h>


//#include "shaders.h"
#include "shader_utils.h"
#include "vector.h"
#include "renderer.h"
#include "lookup.h"




void ce_generate_ansi_16_palette(Color *colors_out) {
    colors_out[0] = (Color)    {0.00f, 0.00f, 0.00f}; // 0: Black
    colors_out[1] = (Color)    {0.50f, 0.00f, 0.00f}; // 1: Dark Red
    colors_out[2] = (Color)    {0.00f, 0.50f, 0.00f}; // 2: Dark Green
    colors_out[3] = (Color)    {0.50f, 0.50f, 0.00f}; // 3: Dark Yellow
    colors_out[4] = (Color)    {0.00f, 0.00f, 0.50f}; // 4: Dark Blue
    colors_out[5] = (Color)    {0.50f, 0.00f, 0.50f}; // 5: Dark Magenta
    colors_out[6] = (Color)    {0.00f, 0.50f, 0.50f}; // 6: Dark Cyan
    colors_out[7] = (Color)    {0.75f, 0.75f, 0.75f}; // 7: Light Gray
    colors_out[8] = (Color)    {0.50f, 0.50f, 0.50f}; // 8: Dark Gray
    colors_out[9] = (Color)    {1.00f, 0.00f, 0.00f}; // 9: Bright Red
    colors_out[10] = (Color)    {0.00f, 1.00f, 0.00f}; // 10: Bright Green
    colors_out[11] = (Color)    {1.00f, 1.00f, 0.00f}; // 11: Bright Yellow
    colors_out[12] = (Color)    {0.00f, 0.00f, 1.00f}; // 12: Bright Blue
    colors_out[13] = (Color)    {1.00f, 0.00f, 1.00f}; // 13: Bright Magenta
    colors_out[14] = (Color)    {0.00f, 1.00f, 1.00f}; // 14: Bright Cyan
    colors_out[15] = (Color)    {1.00f, 1.00f, 1.00f}; // 15: Pure White
}

void ce_generate_xterm_256_palette(Color *colors_out) {
    // indices 0 to 15
    // todo use existing ansi 16
    const float basic_levels[16][3] = {
        {0.00f, 0.00f, 0.00f}, {0.50f, 0.00f, 0.00f}, {0.00f, 0.50f, 0.00f}, {0.50f, 0.50f, 0.00f}, // 0-3
        {0.00f, 0.00f, 0.50f}, {0.50f, 0.00f, 0.50f}, {0.00f, 0.50f, 0.50f}, {0.75f, 0.75f, 0.75f}, // 4-7
        {0.50f, 0.50f, 0.50f}, {1.00f, 0.00f, 0.00f}, {0.00f, 1.00f, 0.00f}, {1.00f, 1.00f, 0.00f}, // 8-11
        {0.00f, 0.00f, 1.00f}, {1.00f, 0.00f, 1.00f}, {0.00f, 1.00f, 1.00f}, {1.00f, 1.00f, 1.00f}  // 12-15
    };
    for (int i = 0; i < 16; i++) {
        colors_out[i].x = basic_levels[i][0];
        colors_out[i].y = basic_levels[i][1];
        colors_out[i].z = basic_levels[i][2];
    }

    // indices 16 to 231
    // levels: 0, 95, 135, 175, 215, 255
    const float cube_steps[6] = { 0.000f, 0.373f, 0.529f, 0.686f, 0.843f, 1.000f };
    int index = 16;
    for (int r = 0; r < 6; r++) {
        for (int g = 0; g < 6; g++) {
            for (int b = 0; b < 6; b++) {
                colors_out[index].x = cube_steps[r];
                colors_out[index].y = cube_steps[g];
                colors_out[index].z = cube_steps[b];
                index++;
            }
        }
    }

    // indices 232 to 255
    for (int i = 0; i < 24; i++) {
        float gray = (8.0f + (float)i * 10.0f) / 255.0f;
        colors_out[index].x = gray;
        colors_out[index].y = gray;
        colors_out[index].z = gray;
        index++;
    }
}




Color ce_get_closest_color(Color in, const Color *colors, size_t num_colors){
    float best_error = 999999.0f;
    Color best_color = {0};

    for (size_t i=0; i<num_colors; i++){
        Color c = colors[i];
        float error = vec3_perceptual_distance(c,in);
        if (error<best_error){
            best_error = error;
            best_color = c;
        }
    }
    return best_color;
}


Color color_to_xterm_save(Color c){
    Color xterm[256];
    ce_generate_xterm_256_palette(xterm);
	return ce_get_closest_color(c,xterm,256);
}

Color color_new_xterm_save_color(float r, float g, float b){
	return color_to_xterm_save( (Color){r,g,b} );
}


// creates a palette based on simple (single byte) characters
// this function does not work with extended ones TODO
AsciiPalette ascii_palette_from_string(char *s, float *coverages)
{
    size_t len = strlen(s);
    AsciiPalette out = {.length=len};
    for (size_t i=0; i<len; i++) {
        out.characters[i][0] = s[i];
        out.characters[i][1] = 0;
        out.characters[i][2] = 0;
    }
    memcpy(out.coverages, coverages, len*sizeof(float));
    return out;
}




// good default values:
// - gamma 1.0f - for blocks
// - use higher values for ascii since they don't have a linear curve
// - color_variance_penalty ~0.15
// - luma_variance_penalty ~0.10
void ce_create_lookup_table(AsciiLUT *lut, const Color colors[], size_t num_colors, AsciiPalette asciiPalette, float gamma, float color_variance_penalty, float luma_variance_penalty)
{
    float density_divisor = (asciiPalette.length > 1) ? (float)(asciiPalette.length - 1) : 1.0f;

    #pragma omp parallel for collapse(3) schedule(dynamic, 32)
    for (int r = 0; r < LUT_SIZE; r++) {
        for (int g = 0; g < LUT_SIZE; g++) {
            for (int b = 0; b < LUT_SIZE; b++)
            {
                float _r = r / (float)(LUT_SIZE-1);
                float _g = g / (float)(LUT_SIZE-1);
                float _b = b / (float)(LUT_SIZE-1);
                Color target_color = {_r, _g, _b};
                target_color = vec3_pow(target_color,gamma);

                float best_error = FLT_MAX;
                AsciiPixel current_best = {0};

                // Loop through characters first to treat density as a structural mix factor
                for (size_t char_index = 0; char_index < asciiPalette.length; char_index++)
                {
                    //float v = (float)char_index / density_divisor;
                    float v = asciiPalette.coverages[char_index];
                    //v = pow(v,2.2);

                    for (size_t i = 0; i < num_colors; i++)
                    {

                        Color color_a = colors[i];
                        float luma_a = luma(color_a);
                        for (size_t j = 0; j < num_colors; j++)
                        {
                            Color color_b = colors[j];
                            float luma_b = luma(color_b);
                            
                            // approximating the perceptual blend of the two colors
                            Color perceived_blend = vec3_lerp(color_b, color_a, v);
                            //float error = vec3_distance2(perceived_blend, target_color);
                            float error = vec3_perceptual_distance(perceived_blend, target_color);

                            float color_variance = vec3_perceptual_distance(color_a, color_b);
                            //float color_variance = vec3_distance2(color_a, color_b);
                            float luma_variance  = fabsf(luma_a - luma_b);

                            float score = error + 
                                          (color_variance * color_variance_penalty) + 
                                          (luma_variance  * luma_variance_penalty);

                            if (score < best_error)
                            {
                                best_error = score;
                                current_best.bg = color_b; 
                                current_best.fg = color_a; 
                                memcpy(current_best.bytes, asciiPalette.characters[char_index], 3 * sizeof(char));
                            }

                        }
                    }
                }
                lut->elements[r][g][b] = current_best;
            }
            
        }
    }
}





// save look-up table to a binary file
bool ce_save_lut_binary(const AsciiLUT *lut, const char *filepath) {
    FILE *file = fopen(filepath, "wb"); // Open in Write-Binary mode
    if (!file) {
        perror("Failed to open LUT file for saving");
        return false;
    }
    size_t written = fwrite(lut->elements, sizeof(AsciiPixel), LUT_SIZE * LUT_SIZE * LUT_SIZE, file);
    fclose(file);
    
    return written == (LUT_SIZE * LUT_SIZE * LUT_SIZE);
}

// load a look-up table from a binary file
bool ce_load_lut_binary(AsciiLUT *lut, const char *filepath) {
    FILE *file = fopen(filepath, "rb"); // Open in Read-Binary mode
    if (!file) {
        perror("Failed to open LUT file for loading");
        return false;
    }
    
    size_t read = fread(lut->elements, sizeof(AsciiPixel), LUT_SIZE * LUT_SIZE * LUT_SIZE, file);
    fclose(file);
    
    return read == (LUT_SIZE * LUT_SIZE * LUT_SIZE);
}

AsciiPixel _lookup_lut(const AsciiLUT *lut, Color color) {
    
    color = vec3_saturate(color);
    // 2. Map 0.0-1.0 to 0-31 integer indices using rounding (+0.5f)
    int idx_r = (int)(color.x * (float)(LUT_SIZE-1) + 0.5f);
    int idx_g = (int)(color.y * (float)(LUT_SIZE-1) + 0.5f);
    int idx_b = (int)(color.z * (float)(LUT_SIZE-1) + 0.5f);

    // 3. Return the precalculated structural element instantly O(1)
    return lut->elements[idx_r][idx_g][idx_b];
}