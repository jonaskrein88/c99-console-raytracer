#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "vector.h"
#include "objects.h"
#include "importer.h"




#define MAX_ROWS 256
#define MAX_COLS 512


static const char BLK_BLANK[3]  = {' ', '\0', '\0'};  // 0%  Density (Space)
static const char BLK_LIGHT[3]  = {0xE2, 0x96, 0x91}; // 25% Density (░)
static const char BLK_MEDIUM[3] = {0xE2, 0x96, 0x92}; // 50% Density (▒)
static const char BLK_DARK[3]   = {0xE2, 0x96, 0x93}; // 75% Density (▓) 
static const char BLK_SOLID[3]  = {0xE2, 0x96, 0x88}; // 100% Density (█)

static const char PX_LETTER_LOW_A[3]= {'a', '\0', '\0'}; // a real byte + 2 padding bytes
static const char PX_LETTER_LOW_I[3]= {'i', '\0', '\0'}; // i real byte + 2 padding bytes

static const char PX_DOT[3]     = {'.', '\0', '\0'}; // . real byte + 2 padding bytes
static const char PX_COMMA[3]   = {',', '\0', '\0'}; // . real byte + 2 padding bytes
static const char PX_COLON[3]   = {':', '\0', '\0'}; // : real byte + 2 padding bytes
static const char PX_SEMI[3]    = {';', '\0', '\0'}; // : real byte + 2 padding bytes
static const char PX_MINUS[3]   = {'-', '\0', '\0'}; // - real byte + 2 padding bytes
static const char PX_EQUAL[3]   = {'=', '\0', '\0'}; // = real byte + 2 padding bytes
static const char PX_PLUS[3]    = {'+', '\0', '\0'}; // + real byte + 2 padding bytes
static const char PX_STAR[3]    = {'*', '\0', '\0'}; // * real byte + 2 padding bytes
static const char PX_HASH[3]    = {'#', '\0', '\0'}; // # real byte + 2 padding bytes
static const char PX_PERCENT[3] = {'%', '\0', '\0'}; // % real byte + 2 padding bytes
static const char PX_AT[3]      = {'@', '\0', '\0'}; // @ real byte + 2 padding bytes

static const char PX_SQUARE[3]      = "■";
static const char PX_LINE[3]        = "─"; 

static const char *CHARACTERS_BLOCK[5] = {BLK_BLANK, BLK_LIGHT, BLK_MEDIUM, BLK_DARK, BLK_SOLID};
//static const char *CHARACTERS_SYMBOLS_10[10] = {BLK_BLANK, PX_DOT,PX_COLON, PX_MINUS, PX_EQUAL, PX_PLUS, PX_LETTER_LOW_I, PX_LETTER_LOW_A ,PX_HASH, PX_AT };



static const Color C_WHITE = {1.f, 1.f, 1.f};
static const Color C_BLACK = {0.f, 0.f, 0.f};
static const Color C_DARK_GREY = {0.3f, 0.3f, 0.3f};
static const Color C_GREY = {0.5f, 0.5f, 0.5f};
static const Color C_LIGHT_GREY = {0.75f, 0.75f, 0.75f};
static const Color C_YELLOW = {1.f, 1.f, 0.f};
static const Color C_GREEN = {0.2f, 1.f, 0.1f};
static const Color C_DARK_GREEN = {0.05f, 0.3f, 0.05f};
static const Color C_DARK_TEAL = {0.025f, 0.1f, 0.1f};
static const Color C_GREEN_CYAN = {0.2f, 1.f, 0.7f};
static const Color C_RED   = {1.f, 0.f, 0.0f};
static const Color C_RED_ORANGE = {1.f, 0.25f, 0.0f};
static const Color C_ORANGE = {1.f, 0.5f, 0.f};
static const Color C_BLUE = {0.2f, 0.1f, 1.f};
static const Color C_TIE_BLUE = {0.35f, 0.65f, 1.f};
static const Color C_DARKBLUE = {0.1f, 0.1f, 0.2f};
static const Color C_LIGHTBLUE = {0.5f, 0.75f, 1.f};
static const Color C_GREYISH_BLUE = {0.5f, 0.65f, 0.85f};

// PICO-8 Color Palette (Normalized Floats)
static const Color C_PICO8_BLACK       = {0.000f, 0.000f, 0.000f}; // 00
static const Color C_PICO8_DARK_BLUE   = {0.114f, 0.169f, 0.325f}; // 01
static const Color C_PICO8_DARK_PURPLE = {0.494f, 0.145f, 0.325f}; // 02
static const Color C_PICO8_DARK_GREEN  = {0.000f, 0.529f, 0.318f}; // 03
static const Color C_PICO8_BROWN       = {0.671f, 0.322f, 0.212f}; // 04
static const Color C_PICO8_DARK_GREY   = {0.373f, 0.341f, 0.310f}; // 05
static const Color C_PICO8_LIGHT_GREY  = {0.761f, 0.765f, 0.780f}; // 06
static const Color C_PICO8_WHITE       = {1.000f, 0.945f, 0.910f}; // 07
static const Color C_PICO8_RED         = {1.000f, 0.000f, 0.302f}; // 08
static const Color C_PICO8_ORANGE      = {1.000f, 0.639f, 0.000f}; // 09
static const Color C_PICO8_YELLOW      = {1.000f, 0.925f, 0.153f}; // 10
static const Color C_PICO8_GREEN       = {0.000f, 0.894f, 0.212f}; // 11
static const Color C_PICO8_BLUE        = {0.161f, 0.678f, 1.000f}; // 12
static const Color C_PICO8_LAVENDER    = {0.514f, 0.463f, 0.612f}; // 13
static const Color C_PICO8_PINK        = {1.000f, 0.467f, 0.659f}; // 14
static const Color C_PICO8_LIGHT_PEACH = {1.000f, 0.800f, 0.667f}; // 15

// ANSII palette
static const Color C_ANSI_BLACK           =   {0.00f, 0.00f, 0.00f}; // 0: Black
static const Color C_ANSI_DARK_RED        =   {0.50f, 0.00f, 0.00f}; // 1: Dark Red
static const Color C_ANSI_DARK_GREEN      =   {0.00f, 0.50f, 0.00f}; // 2: Dark Green
static const Color C_ANSI_DARK_YELLOW     =   {0.50f, 0.50f, 0.00f}; // 3: Dark Yellow
static const Color C_ANSI_DARK_BLUE       =   {0.00f, 0.00f, 0.50f}; // 4: Dark Blue
static const Color C_ANSI_DARK_MAGENTA    =   {0.50f, 0.00f, 0.50f}; // 5: Dark Magenta
static const Color C_ANSI_DARK_CYAN       =   {0.00f, 0.50f, 0.50f}; // 6: Dark Cyan
static const Color C_ANSI_LIGHT_GREY      =   {0.75f, 0.75f, 0.75f}; // 7: Light Gray
static const Color C_ANSI_DARK_GREY       =   {0.50f, 0.50f, 0.50f}; // 8: Dark Gray
static const Color C_ANSI_BRIGHT_RED      =   {1.00f, 0.00f, 0.00f}; // 9: Bright Red
static const Color C_ANSI_BRIGHT_GREEN    =   {0.00f, 1.00f, 0.00f}; // 10: Bright Green
static const Color C_ANSI_BRIGHT_YELLOW   =   {1.00f, 1.00f, 0.00f}; // 11: Bright Yellow
static const Color C_ANSI_BLUE            =   {0.00f, 0.00f, 1.00f}; // 12: Bright Blue
static const Color C_ANSI_MAGENTA         =   {1.00f, 0.00f, 1.00f}; // 13: Bright Magenta
static const Color C_ANSI_BRIGHT_CYAN     =   {0.00f, 1.00f, 1.00f}; // 14: Bright Cyan
static const Color C_ANSI_WHITE           =   {1.00f, 1.00f, 1.00f}; // 15: Pure White

static const Color *C_ANSI_PALETTE[] = {
    &C_ANSI_BLACK,
    &C_ANSI_DARK_RED,
    &C_ANSI_DARK_GREEN,
    &C_ANSI_DARK_YELLOW,
    &C_ANSI_DARK_BLUE,
    &C_ANSI_DARK_MAGENTA,
    &C_ANSI_DARK_CYAN,
    &C_ANSI_LIGHT_GREY,
    &C_ANSI_DARK_GREY,
    &C_ANSI_BRIGHT_RED,
    &C_ANSI_BRIGHT_GREEN,
    &C_ANSI_BRIGHT_YELLOW,
    &C_ANSI_BLUE,
    &C_ANSI_MAGENTA,
    &C_ANSI_BRIGHT_CYAN,
    &C_ANSI_WHITE
};


// circular dependency 
typedef struct Object Object; 
typedef struct AsciiLUT AsciiLUT; 
typedef enum EnvironmentProjectionMethod EnvironmentProjectionMethod;



typedef struct{
    char bytes[3];
    Color fg;
    Color bg;
} AsciiPixel;


typedef struct {
    AsciiPixel *pixels;
    int width;
    int height;
} AsciiCanvas;


typedef struct Ray {
    Vec3 origin;
    Vec3 direction;
} Ray;

typedef struct {
	Vec3 up;
	Vec3 forward;
	Vec3 right;
	Vec3 pos;
	Vec3 target;
	float focalDist;
    float sensorWidth;
    float sensorHeight;
    float sensorRatio;
    float ortho_size;
    float yaw;
    float pitch;
    bool is_targeted;
    bool is_orthographic;
} Camera;





typedef struct{
    Vec3 center;
    float radius;
}Sphere;


typedef enum {
    LIGHT_POINT,
    LIGHT_DIRECTIONAL
} LightType;

typedef struct{
    LightType type;
    Vec3 pos;
    float power;
    Color color;
}Light;


typedef struct{
    Object *objects[64];
    Light *lights[8];
    size_t numObjects;
    size_t numLights;
}Scene;


static uint8_t NUM_RENDER_MODES = 3;
typedef enum {
    RENDERMODE_DEFAULT,
    RENDERMODE_DEBUG,
    RENDERMODE_GREYBOX
} RenderMode;

typedef struct Material Material;

typedef struct RenderSettings{
    
    uint16_t current_frame;
    uint8_t render_width;
    uint8_t render_height;
    RenderMode render_mode;
    Camera camera;

    AsciiCanvas canvas;
    AsciiCanvas _temp_canvas;
    float   *depth_buffer;
    uint8_t *stencil_buffer;

    Scene scene;
    Color background_color;

    TextureImage *textures[64];
    size_t _num_textures;

    Mesh **meshes;
    size_t num_meshes;

    Material *environment;
    Material *materials;
    Material *_grey_box_material;
    bool render_shadows;
    bool fog_enabled;
    float fog_start;
    float fog_end;

} RenderSettings;


typedef struct HitResult{
    bool did_hit;
    float depth;
    Normal normal;
    Object *object;
    Face *face;
    Vec3 barycentric_weights;
    Ray ray;
}HitResult;


typedef struct {
    Normal normal;
    Color color;
    float depth;
    Vec3 pos;
    Vec3 camera_position;
    Vec3 view_direction;
    uint8_t x_coord;
    uint8_t y_coord;
    Vec3 uv;
} FragmentData;


typedef struct Material {
    void(*shader)(RenderSettings*, FragmentData*, void*, AsciiPixel*);
    void* parameters;
    uint8_t stencil;
} Material;



typedef enum BlendModeColor {
    BLEND_METHOD_COLOR_NO_OP,
    BLEND_METHOD_COLOR_OVERWRITE,
    BLEND_METHOD_COLOR_ADD,
    BLEND_METHOD_COLOR_VIVID_LIGHT,
    BLEND_METHOD_COLOR_MULTIPLY
} BlendMode_Color;

typedef enum BlendModeAscii {
    BLEND_METHOD_ASCII_NO_OP,
    BLEND_METHOD_ASCII_FILL
} BlendMode_Ascii;


typedef enum DitherPattern {
    DITHER_PATTERN_BAYER_4X4,
    DITHER_PATTERN_BAYER_4X4_BINARY,
    DITHER_PATTERN_BAYER_1X4,
    DITHER_PATTERN_BAYER_1X4_BINARY,
    DITHER_PATTERN_BAYER_4X1,
    DITHER_PATTERN_BAYER_4X1_BINARY,
    DITHER_PATTERN_BAYER_2X2,
    DITHER_PATTERN_BAYER_2X2_BINARY,
} DitherPattern;



void camera_update_transform(Camera *cam);
void camera_init(Camera *cam, uint8_t width, uint8_t height);
void camera_walk(Camera *cam, float value);
void camera_strafe(Camera *cam, float value);
void camera_strafe_vertical(Camera *cam, float value);
void camera_zoom(Camera *cam, float value);
void camera_rotate_yaw(Camera *cam, float value);
void camera_rotate_pitch(Camera *cam, float value);
bool camera_toggle_orthographic_projection(Camera *cam);
Matrix camera_get_transform(Camera *cam);
Ray camera_ray(Camera *cam, float u, float v);

Light ce_new_point_light(float power, Color color, Vec3 pos);
Light ce_new_directional_light(float power, Color color, float height, float angle);



AsciiPixel ce_new_ascii_pixel_three_bytes(Color bg, Color fg, const char *bytes);
AsciiPixel ce_new_ascii_pixel_single_byte(Color bg, Color fg, const char *symbol);


size_t ce_import_texture(RenderSettings *renderSettings, char *filepath);

//void   ce_remap_indexed_texture(RenderSettings *renderSettings, Color colors[], uint8_t num_colors, size_t tex_id);



bool calculate_shadow(RenderSettings *renderSettings, Vec3 position, Vec3 lightDir);
bool cast_reflection_ray(RenderSettings *renderSettings, Vec3 position, Vec3 normal, Vec3 view_direction, HitResult *out_result);


/*
Material ce_new_material_retro(Color color, float spec_amount, float dither_amount);
Material ce_new_material_retro_textured(Color color, float spec_amount, float dither_amount, uint8_t tex_id);

Material ce_new_material_retro_two_tone(         Color bg, Color fg, float spec_amount, float dither_amount);
Material ce_new_material_retro_two_tone_textured(Color bg, Color fg, float spec_amount, float dither_amount, uint8_t tex_id);
*/

Material ce_new_material_gradient_ramp(         Color *colors, uint8_t num_colors, float spec_amount, float dither_amount, const char **symbols, uint8_t num_symbols);
Material ce_new_material_gradient_ramp_interpolated( AsciiLUT *lut,Color *colors, uint8_t num_colors, float spec_amount, float dither_amount);
Material ce_new_material_gradient_ramp_blocks         (Color *colors, uint8_t num_colors, float spec_amount, float dither_amount);

void ce_gradient_material_set_brightness(Material *mtl, float brightness);
void ce_gradient_material_set_ambient(Material *mtl, float ambient);
void ce_gradient_material_set_specular_sharpness(Material *mtl, float sharpness);
void ce_gradient_material_set_texture(Material *mtl, uint8_t tex_id);
void ce_gradient_material_set_dither_pattern(Material *mtl, DitherPattern dither_pattern);



Material ce_new_material_tiles(AsciiPixel *tiles, size_t num_tiles, float spec_amount, float dither_amount);
void ce_tile_material_set_brightness(Material *mtl, float brightness);
void ce_tile_material_set_texture(Material *mtl, uint8_t tex_id);

Material ce_new_material_chrome(AsciiLUT *lut, Color tint, float dither_amount, uint8_t tex_id);


Material ce_new_material_principled(AsciiLUT *lut, Color color, float spec_amount, float dithering);
Material ce_new_material_full(Color color, float spec_amount);
void ce_principled_material_set_texture(Material *mtl, uint8_t tex_id);
void ce_principled_material_set_reflection(Material *mtl, float mult, float fresnel);
void ce_principled_material_set_ambient(Material *mtl, float amount);
void ce_principled_material_set_rim_light(Material *mtl, float amount);


Material ce_new_material_post_lut(AsciiLUT *lut, float dither_amount);
Material ce_new_material_post_bloom(AsciiLUT *lut, float dither_amount);
Material ce_new_material_post_blur(AsciiLUT *lut, size_t x_samples, size_t y_samples, float dither_amount);
Material ce_new_material_post_dispersion(AsciiLUT *lut, size_t num_samples, size_t size, float dither_amount);
Material ce_new_material_post_horizontal_glitches(AsciiLUT *lut, size_t num_samples, size_t stride, float dropout_chance,float dither_amount);
Material ce_new_material_post_horizontal_blur(AsciiLUT *lut, size_t num_samples, size_t stride, float dropout_chance,float dither_amount);

Material ce_new_material_environment_texture(AsciiLUT *lut, EnvironmentProjectionMethod method, float brightness, float dither_amount, uint8_t tex_id);

static bool render_pixel(RenderSettings *renderSettings, uint8_t x, uint8_t y, AsciiPixel *out_pixel);

RenderSettings *ce_create_rendersettings();
void ce_add_object_to_scene(RenderSettings *renderSettings, Object *obj);
void ce_add_light_to_scene(RenderSettings *renderSettings, Light *light);


bool process_ray(RenderSettings *renderSettings, size_t x_coord, size_t y_coord, HitResult *hit_result, AsciiPixel *out_pixel);
void ce_render_frame(RenderSettings *renderSettings);

void cycle_modes(RenderSettings *settings);

int get_key_async();
void ce_object_manual_turntable(Object *obj);
void ce_clear_canvas(RenderSettings *settings);
bool ce_play(RenderSettings *settings);
void ce_display_canvas(RenderSettings *renderSettings);
//void free_renderer(RenderSettings *renderSettings);