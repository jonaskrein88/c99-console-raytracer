

#include <stdint.h>
#include <math.h>
#include <float.h>
#include "vector.h"
#include "renderer.h"
#include "shader_utils.h"
#include "lookup.h"




float clamp01(float x){
    return fminf(1.0f, fmaxf(0.f,x));
}

float remap_to01(float value, float originalMin, float originalMax) {
    if (fabsf(originalMax - originalMin) < 0.00001f) {
        return 0.0f; 
    }
    return clamp01((value - originalMin) / (originalMax - originalMin));
}

float remap_from01(float value, float min, float max) {
    value = clamp01(value);
    return value *(max-min) + min;
}



float hash11(float p) {
    // Scramble the input with a high frequency sine wave
    float s = sinf(p * 127.1f) * 43758.5453123f;
    // Extract the fractional part to guarantee a 0.0 to 1.0 range
    return s - floorf(s);
}

Vec2 hash22(Vec2 p) {
    float d1 = p.x * 127.1f + p.y * 311.7f;
    float d2 = p.x * 269.5f + p.y * 183.3f;
    float s1 = sinf(d1) * 43758.5453123f;
    float s2 = sinf(d2) * 43758.5453123f;
    Vec2 out_hash;
    out_hash.x = s1 - floorf(s1);
    out_hash.y = s2 - floorf(s2);
    return out_hash;
}

Vec3 hash33(Vec3 p) {
    // 3D dot product constants to mix all three structural planes
    float d1 = p.x * 127.1f + p.y * 311.7f + p.z * 74.7f;
    float d2 = p.x * 269.5f + p.y * 183.3f + p.z * 246.1f;
    float d3 = p.x * 113.5f + p.y * 271.9f + p.z * 124.6f;
    float s1 = sinf(d1) * 43758.5453123f;
    float s2 = sinf(d2) * 43758.5453123f;
    float s3 = sinf(d3) * 43758.5453123f;
    Vec3 out_hash;
    out_hash.x = s1 - floorf(s1);
    out_hash.y = s2 - floorf(s2);
    out_hash.z = s3 - floorf(s3);
    return out_hash;
}















float luma(Color c) {
    return vec3_dot(c, (Vec3){0.299f,0.587f,0.114f});
}

float lerpf(float a, float b, float t){
    return b * t + a * (1.f-t);
}

Color rgb2hsv(Color in)
{
    Color out;
    float min, max, delta;

    min = MIN(MIN(in.x, in.y), in.z);
    max = MAX(MAX(in.x, in.y), in.z);
    delta = max - min;

    out.z = max; // v

    if (max <= 0.0 || delta < 0.00001)
    {
        out.y = 0.0;
        out.x = 0.0; // 0.0 is safer than NAN for pipeline stability
        return out;
    }

    out.y = delta / max; // s
    if (in.x >= max)
    {
        out.x = (in.y - in.z) / delta;
        if (out.x < 0.0) out.x += 6.0; // Keeps hue positive before scale
    }
    else if (in.y >= max)
    {
        out.x = 2.0 + (in.z - in.x) / delta;
    }
    else
    {
        out.x = 4.0 + (in.x - in.y) / delta;
    }

    out.x *= 60.0; // degrees
    return out;
}

Color hsv2rgb(Color in)
{
    Color out;
    if (in.y <= 0.0)
    {
        out.x = in.z;
        out.y = in.z;
        out.z = in.z;
        return out;
    }
    float h = in.x / 60.0;

    float r_raw = fabs(h - 3.0) - 1.0;
    float g_raw = 2.0 - fabs(h - 2.0);
    float b_raw = 2.0 - fabs(h - 4.0);

    float r = CLAMP(r_raw, 0.0, 1.0);
    float g = CLAMP(g_raw, 0.0, 1.0);
    float b = CLAMP(b_raw, 0.0, 1.0);

    out.x = ((r - 1.0) * in.y + 1.0) * in.z;
    out.y = ((g - 1.0) * in.y + 1.0) * in.z;
    out.z = ((b - 1.0) * in.y + 1.0) * in.z;

    return out;
}

Color dither(Color color, int x, int y, float amount) // dither and saturate
{
	x = x % 4;
	y = y % 4;
	float d = BAYERMATRIX[x + y * 4] / 16.f;
    d -= 0.5f;
    d*=amount;
    Color dithered = (Color) {color.x+d, color.y+d, color.z+d};
    return vec3_saturate(dithered);
}

float dither_float(float v, int x, int y, float amount, DitherPattern dither_pattern)
{
    float threshold;
    switch (dither_pattern)
    {
    case DITHER_PATTERN_BAYER_1X4:
        y = y % 4;
        threshold = BAYER1x4[y] / 4.f;
        return v+(threshold-0.5f)*amount;

    case DITHER_PATTERN_BAYER_4X1:
        x = x % 4;
        threshold = BAYER1x4[x] / 4.f;
        return v+(threshold-0.5f)*amount;

    case DITHER_PATTERN_BAYER_4X4_BINARY:
        x = x % 4;
        y = y % 4;
        threshold = BAYERMATRIX[x + y * 4] / 16.f;
        return v>threshold;

    case DITHER_PATTERN_BAYER_1X4_BINARY:
        y = y % 4;
        threshold = BAYER1x4[y] / 4.f;
        return 0.5f>threshold;

    case DITHER_PATTERN_BAYER_4X1_BINARY:
        x = x % 4;
        threshold = BAYER1x4[x] / 4.f;
        return v>threshold;

    default:
        x = x % 4;
        y = y % 4;
        threshold = BAYERMATRIX[x + y * 4] / 16.f;
        return v+(threshold-0.5f)*amount;
    }
}











// maps an input brightness to a palette
// returns the two colors and their mixing ratio
float gradient_map(float value, const Color *colors, size_t num_colors, Color *out_color_a, Color *out_color_b) {
    value = clamp01(value);
   
    // just output the two colors if it's only them
    if (num_colors==2){
        *out_color_a = colors[0];
        *out_color_b = colors[1];
        return value;
    }
    float scaled = value * (float)(num_colors - 1);
    
    size_t index = (size_t)floorf(scaled);
    float t = scaled - (float)index; // faster and safer than fmodf
    
    size_t next = index + 1;
    if (next >= num_colors) {
        next = num_colors - 1; 
    }
    *out_color_a = colors[index]; 
    *out_color_b = colors[next]; 
    return t;
}


Color gradient_map_interpolated(float value, const Color *colors, size_t num_colors){
   Color a,b;
   float t = gradient_map(value,colors,num_colors,&a,&b); 
   return vec3_lerp(a,b,t);
}





float calculate_fresnel(Vec3 normal, Vec3 view_dir, float base_reflectivity) {
    float cos_theta = fmaxf(0.0f, vec3_dot(normal, view_dir));
    float x = 1.0f - cos_theta;
    float x5 = x * x * x * x * x; // Faster than calling powf(x, 5.0f)
    return base_reflectivity + (1.0f - base_reflectivity) * x5;
}




Color sample_texture_bilinear(RenderSettings *renderSettings, size_t textureIndex, float u, float v)
{
    TextureImage *tex = renderSettings->textures[textureIndex];
    if (tex == NULL) {
        return (Color) {1.0f, 0.0f, 0.0f}; // Error red
    }

    // 1. Repeating texture wrap (tiling wrapper)
    u = u - floorf(u);
    v = v - floorf(v);

    // 2. Map coordinates to continuous floating-point texel space
    // We subtract 0.5f because pixel centers sit at half-steps
    float texel_x = (u * (float)tex->width) - 0.5f;
    float texel_y = (v * (float)tex->height) - 0.5f;

    // 3. Extract the base integer coordinate floor
    int x0 = (int)floorf(texel_x);
    int y0 = (int)floorf(texel_y);

    // 4. Calculate the fractional weights (how far we are between pixels)
    float weight_x = texel_x - (float)x0;
    float weight_y = texel_y - (float)y0;

    // 5. Determine neighbors with strict clamping boundary security
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    if (x0 < 0) x0 = 0; if (x0 >= tex->width)  x0 = tex->width - 1;
    if (x1 < 0) x1 = 0; if (x1 >= tex->width)  x1 = tex->width - 1;
    if (y0 < 0) y0 = 0; if (y0 >= tex->height) y0 = tex->height - 1;
    if (y1 < 0) y1 = 0; if (y1 >= tex->height) y1 = tex->height - 1;

    // 6. Fetch the 4 surrounding neighbor pixels
    Color p00 = tex->pixels[x0 + y0 * tex->width]; // Top-Left
    Color p10 = tex->pixels[x1 + y0 * tex->width]; // Top-Right
    Color p01 = tex->pixels[x0 + y1 * tex->width]; // Bottom-Left
    Color p11 = tex->pixels[x1 + y1 * tex->width]; // Bottom-Right

    // 7. THE BILINEAR BLEND:
    // Blend horizontally across the top row
    Color top_blend    = vec3_lerp(p00, p10, weight_x);
    // Blend horizontally across the bottom row
    Color bottom_blend = vec3_lerp(p01, p11, weight_x);
    // Blend vertically between the two row results
    Color final_rgb    = vec3_lerp(top_blend, bottom_blend, weight_y);

    // 8. Return formatted to your system's Color struct specifications
    return (Color){
        (float)final_rgb.x,
        (float)final_rgb.y,
        (float)final_rgb.z
    };
}



Color sample_texture(RenderSettings *renderSettings, size_t textureIndex, float u, float v)
{
    TextureImage *tex = renderSettings->textures[textureIndex];
    if (tex == NULL) {
        // FIX: Changed %d to %zu for correct size_t string representation
        printf("No texture to sample at %zu\n", textureIndex); 
        return (Color) {1.0f, 0.0f, 1.0f}; // Error purple
    }
    u = u - floorf(u);
    v = v - floorf(v);
    int tex_x = (int)(u * (float)tex->width);
    int tex_y = (int)(v * (float)tex->height);
    if (tex_x < 0) tex_x = 0;
    if (tex_x >= tex->width)  tex_x = tex->width - 1;
    if (tex_y < 0) tex_y = 0;
    if (tex_y >= tex->height) tex_y = tex->height - 1;
    size_t texel = (size_t)tex_x + ((size_t)tex_y * (size_t)tex->width);
    return tex->pixels[texel];
}












Color sample_matcap(RenderSettings *settings, size_t index, Normal N) {
    float dot_right = vec3_dot(N, settings->camera.right);
    float dot_up    = vec3_dot(N, settings->camera.up);

    float u = dot_right * 0.5f + 0.5f;
    float v = dot_up    * 0.5f + 0.5f;
    Color tex_color = sample_texture_bilinear(settings, index, u,v);
    return tex_color;
}





Color sample_environmnet(RenderSettings *renderSettings, Vec3 view, size_t texture)
{
    float phi = atan2(view.z, view.x);
    float theta = asin(view.y);
    float u = 1.0 - (phi + PI) / (2.0 * PI);
    float v = (theta + PI/2.0) / PI;
    return sample_texture_bilinear(renderSettings, texture, u,v);
}


uint8_t sample_stencil_buffer(RenderSettings *renderSettings, int16_t x, int16_t y){
    x = MAX(x,0);
    y = MAX(y,0);
    x = MIN(x,renderSettings->canvas.width-1);
    y = MIN(y,renderSettings->canvas.height-1);
    return renderSettings->stencil_buffer[x + y*renderSettings->canvas.width];
}


// returns a pointer to the ascii pixel in the canvas;
AsciiPixel *sample_canvas(RenderSettings *renderSettings, int16_t x, int16_t y){
    x = MAX(x,0);
    y = MAX(y,0);
    x = MIN(x,renderSettings->canvas.width-1);
    y = MIN(y,renderSettings->canvas.height-1);
    return &renderSettings->canvas.pixels[x + y*renderSettings->canvas.width];
}








float calculate_fog(RenderSettings *settings, FragmentData *frag)
{
    if (!settings->fog_enabled) return 1.0f;
    float fog_start = settings->fog_start;
    float fog_end   = settings->fog_end;

    float fog = (fog_end - frag->depth) / (fog_end - fog_start);

    return fmaxf(0.f, fminf(fog, 1.f));
}




static inline uint8_t _vertical_mask(int y, float main_freq, float fm_freq, float fm_amount){
    float offset = cos(fm_freq * y) * fm_amount;
    float s = sin(main_freq * y + offset);
    s = s*2.0f-1.0f;
    return abs(roundf(s));

}

static inline float _glitch_bayer16(int x, int y){
	x = (y+x) % 4;
	y = y % 4;
	float out = BAYERMATRIX[x + y * 4] / 16.f;

    return out;
}
static inline float _glitch_bayer4(int x, int y){
    static const float BAYER4X4[4] = {0,1,2,3};
	x = (y+x) % 2;
	y = y % 2;
	float out = BAYER4X4[x + y * 2] / 4.f;
    return out;
}
static inline float _glitch_vertical_lines(int x, int y) {
    //static const float BAYER4x1[4] = {0,3,1,2};
    static const float GRADIENT4x1[4] = {0,1,2,3};
	x = x % 4;
	float out = GRADIENT4x1[x] / 4.f;
    return out;
}
static inline float _glitch_bayer4x1(int x, int y){
    static const float BAYER4x1[4] = {0,3,1,2};
	x = x % 4;
	float out = BAYER4x1[x] / 4.f;
    return out;
}



static inline float _glitch_horizontal_lines(int x, int y) {
    return _glitch_vertical_lines(y,x);
}
static inline float _glitch_diagonal_lines_down(int x, int y) {
    return _glitch_vertical_lines(x+y,y);
}
static inline float _glitch_diagonal_lines_up(int x, int y) {
    return _glitch_vertical_lines(x-y,y);
}

// nice pattern but no dynamic range TODO
static inline float _glitch_irregular_grid(int x, int y){
    uint8_t block_size = 4;
    int swap_mask = (x & block_size) / block_size; 
    int y_frequency = (y & (block_size * 2)) / (block_size * 2);
    swap_mask = swap_mask ^ y_frequency; 
    float a = _glitch_bayer4x1(x,y);
    float b = _glitch_bayer4(x,y);
    return lerpf(a,b,swap_mask);
}
static inline float _glitch_grid_line_mix(int x, int y){
    int check = (x % 2) ^ (y % 2); // Base checkerboard (0 or 1)
    int vert_lines = (x % 2);                   // Base vertical lines (0 or 1)
    int mask = round(_vertical_mask(y, 8.1f, 2.2f,0.1f));
    float final = lerpf(check, vert_lines, mask);
    return clamp01(final);
}
static inline float _glitch_checker(int x, int y){
    int check = (x % 2) ^ (y % 2); // Base checkerboard (0 or 1)
    return check;
}


Color make_glitchy_dithering(Color in, size_t x, size_t y, float dither_amount)
{

    float l = luma(in);
    //l = fmodf(l*2.0f, 1.0f);

    // debug with a screen space gradient
    //l = x / renderSettings->render_width;
    
    uint8_t num_dither_methods = 3;
    uint8_t index = floorf(l*num_dither_methods-0.01f);
    
    // construct our weird dither pattern
    float threshold_map = 0;
    switch (index) 
    {
        //case 0: threshold_map = _glitch_irregular_grid(x,y); break;
        
        case 0: threshold_map = _glitch_bayer16(x,y); break;
        case 1: threshold_map = _glitch_bayer4(x,y); break;
        case 2: threshold_map = _glitch_grid_line_mix(x,y); break;
        default: threshold_map = _glitch_bayer16(x,y); break;
    }
    //return new_mono(threshold_map);
    //threshold_map = _glitch_diagonal_lines(x,y);
    //threshold_map = _glitch_bayer16(x,y);
    //threshold_map = _glitch_bayer4(x,y);
    //threshold_map = _glitch_grid_line_mix(x,y);
    threshold_map -= 0.5f; 
    in.x += threshold_map * dither_amount;
    in.y += threshold_map * dither_amount;
    in.z += threshold_map * dither_amount;
    return in;
}




static inline float vivid_light_channel(float target, float blend) {
    if (blend <= 0.5f) {
        // Color Burn branch
        float denominator = 2.0f * blend;
        if (denominator < 1e-6f) denominator = 1e-6f; // Prevent division by zero
        return clamp01(1.0f - ((1.0f - target) / denominator));
    } else {
        // Color Dodge branch
        float denominator = 2.0f * (1.0f - blend);
        if (denominator < 1e-6f) denominator = 1e-6f; // Prevent division by zero
        return clamp01(target / denominator);
    }
}

static inline Color blend_vivid_light(Color target, Color blend) {
    Color result;
    result.x = vivid_light_channel(target.x, blend.x);
    result.y = vivid_light_channel(target.y, blend.y);
    result.z = vivid_light_channel(target.z, blend.z);
    return result;
}

Color blend(Color base, Color blend, BlendMode_Color mode)
{
    switch (mode)
    {
        case BLEND_METHOD_COLOR_NO_OP:              return base;
        case BLEND_METHOD_COLOR_OVERWRITE:          return blend;
        case BLEND_METHOD_COLOR_MULTIPLY:           return vec3_mul_vec3(base, blend);
        case BLEND_METHOD_COLOR_ADD:                return vec3_add(base, blend);
        case BLEND_METHOD_COLOR_VIVID_LIGHT:        return blend_vivid_light(base, blend);
    }
}


static inline const char *_get_char_from_float(const char **palette, uint8_t num_symbols, float value)
{
    value = clamp01(value);
    int index = (int)(value * (num_symbols-0.01));
    return palette[index];
}

void _replace_only_empty_char(AsciiPixel *out_pixel, const char* bytes)
{
    if (out_pixel->bytes[0] == ' ' || out_pixel->bytes == BLK_SOLID )  memcpy(out_pixel->bytes, bytes, 3*sizeof(char));
}


// draws the color in back and foreground
// SPACE ascii symbol
void _ascii_shader_full(Color color, AsciiPixel *out_pixel){
    out_pixel->fg = color;
    out_pixel->bg = color;
    memcpy(out_pixel->bytes, BLK_BLANK, sizeof(char)*3);
}

// write to the ascii pixel
// fetches symbol from list based on luma
void _ascii_shader_from_symbol_list(Color fg, Color bg, const char **symbols, size_t num_symbols, float luma, AsciiPixel *out_pixel){
    const char* symbol = _get_char_from_float(symbols,num_symbols,luma);
    out_pixel->fg = fg;
    out_pixel->bg = bg;
    memcpy(out_pixel->bytes, symbol, sizeof(char)*3);
}


// takes the color and brightness already seperated
// usually for debug shader or for passing full colors to post process
void _ascii_shader_blocks(Color fg, Color bg, float luma, AsciiPixel *out_pixel)
{
    const char *palette[4] = {BLK_BLANK, BLK_LIGHT, BLK_MEDIUM, BLK_SOLID};

    const char* symbol = _get_char_from_float(palette,4,luma);
    out_pixel->fg = fg;
    out_pixel->bg = bg;
    memcpy(out_pixel->bytes, symbol, sizeof(char)*3);
}


// writes the result of the lut look up to the ascii pixel
void _ascii_shader_from_lut(Color color, AsciiLUT *lut, AsciiPixel *out_pixel){
    AsciiPixel best = _lookup_lut(lut, color);
    out_pixel->fg = best.fg;
    out_pixel->bg = best.bg;
    memcpy(out_pixel->bytes, best.bytes, 3*sizeof(char));
}