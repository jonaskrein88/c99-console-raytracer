#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "renderer.h"
#include "vector.h"
#include "importer.h"
#include "objects.h"
#include "shaders.h" 
#include "graphics2D.h" 
#include "raymarching.h" 
#include "lookup.h" 













int main() {


    
    RenderSettings *settings = ce_create_rendersettings();
    settings->background_color = C_BLACK;
    uint16_t w = settings->render_width;
    uint16_t h = settings->render_height;





    int8_t tex_metalcap = ce_import_texture(settings,"textures/metalCap.bmp");
    int8_t tex_sky_lava = ce_import_texture(settings,"textures/sm64_lava.bmp");
    int8_t tex_sky_dark = ce_import_texture(settings,"textures/sm64_skyDark.bmp");
    int8_t tex_sky_space = ce_import_texture(settings,"textures/space.bmp");
    int8_t tex_sky_gradient_blue = ce_import_texture(settings,"textures/gradient_blue.bmp");

   
    // example of creating an ascii charcter palette for LUT generation
    // since the ascii sequence is most likely not linear we need to supply some coverage values
    float coverage[] = {0.0f, 0.05f, 0.11f, 0.18f, 0.31f, 0.42f, 0.58f, 0.72f, 0.86f, 1.0f};
    const AsciiPalette symbols_9 = ascii_palette_from_string(" .:-=+*#%@", coverage);
    




    // example creating and exporting the xTerm 256 LUT
    #if 0
    {
    Vec3 xterm_colors[256];
    AsciiLUT *lut_xterm_256;
    lut_xterm_256 = calloc(1,sizeof(AsciiLUT));
    ce_generate_xterm_256_palette(xterm_colors);
    printf("created xterm palette\n");
    ce_create_lookup_table(lut_xterm_256 ,xterm_colors, 256, CHARACTERS_BLOCK, 5,1.0f, 0.075f, 0.05f);
    printf("created xterm table\n");
    ce_save_lut_binary(lut_xterm_256, "models/lut_xterm_256.bin");
    printf("exported xterm table\n");
    free(lut_xterm_256);
    }
    #endif

    // xterm 256 LUT symbols
    #if 0 
    Vec3 xterm_colors[256];
    AsciiLUT *_lut;
    _lut = calloc(1,sizeof(AsciiLUT));
    ce_generate_xterm_256_palette(xterm_colors);
    printf("created xterm palette\n");
    float color_penalty = 0.05;
    float luma_penalty = 0.025;
    ce_create_lookup_table(_lut ,xterm_colors, 256, symbols_9, 1.0f, color_penalty, luma_penalty);
    printf("created xterm table\n");
    ce_save_lut_binary(_lut, "models/lut_xterm_256_symbols.bin");
    printf("exported xterm table\n");
    free(_lut);
    //return 0;
    #endif

    #if 0 
    Vec3 xterm_colors[256];
    AsciiLUT *_lut;
    _lut = calloc(1,sizeof(AsciiLUT));
    ce_generate_xterm_256_palette(xterm_colors);
    printf("created xterm palette\n");
    float color_penalty = 0.0;
    float luma_penalty = 0.01;
    ce_create_lookup_table(_lut ,xterm_colors, 256, symbols_9, 1.0f, color_penalty, luma_penalty);
    printf("created xterm table\n");
    ce_save_lut_binary(_lut, "models/lut_xterm_256_symbols_vibrant.bin");
    printf("exported xterm table\n");
    free(_lut);
    //return 0;
    #endif
    


    // ansi 16 LUT w Blocks
    #if 0
    {
    Color ansi_16_colors[16];
    AsciiLUT *lut_ansi_16;
    lut_ansi_16 = calloc(1,sizeof(AsciiLUT));
    ce_generate_ansi_16_palette(ansi_16_colors);
    //size_t num_symbols = sizeof(CHARACTERS_BLOCK)/sizeof(CHARACTERS_BLOCK[0]);
    ce_create_lookup_table(lut_ansi_16, ansi_16_colors, 16, CHARACTERS_BLOCK, 5);
    ce_save_lut_binary(lut_ansi_16, "models/lut_ansi_16.bin");
    free(lut_ansi_16);
    printf("Lut ansi 16 exported successfully");
    }
    #endif
    

    // ansi 16 LUT w symbols
    #if 0 
    Color ansi_16_colors[16];
    ce_generate_ansi_16_palette(ansi_16_colors);
    AsciiLUT *_lut;
    _lut = calloc(1,sizeof(AsciiLUT));
    float color_penalty = 0.05;
    float luma_penalty = 0.025;
    ce_create_lookup_table(_lut ,ansi_16_colors, 16, symbols_9, 1.0f, color_penalty, luma_penalty);
    ce_save_lut_binary(_lut, "models/lut_ansi_16_symbols.bin");
    free(_lut);
    #endif
    







    AsciiLUT *lut_xterm_256                 = calloc(1,sizeof(AsciiLUT));
    AsciiLUT *lut_xterm_256_symbols         = calloc(1,sizeof(AsciiLUT));
    AsciiLUT *lut_xterm_256_symbols_vibrant = calloc(1,sizeof(AsciiLUT));
    AsciiLUT *lut_ansi_16                   = calloc(1,sizeof(AsciiLUT));
    AsciiLUT *lut_ansi_16_symbols           = calloc(1,sizeof(AsciiLUT));

    ce_load_lut_binary(lut_xterm_256,         "models/lut_xterm_256.bin");
    ce_load_lut_binary(lut_xterm_256_symbols, "models/lut_xterm_256_symbols.bin");
    ce_load_lut_binary(lut_xterm_256_symbols_vibrant, "models/lut_xterm_256_symbols_vibrant.bin");
    ce_load_lut_binary(lut_ansi_16,           "models/lut_ansi_16.bin");
    ce_load_lut_binary(lut_ansi_16_symbols,   "models/lut_ansi_16_symbols.bin");

    #if 0
    Mesh m_bowser = import_obj("bowser.obj");
    Object bowser = new_object(&m_bowser);
    object_assign_material(&bowser, &yellow, 0);
    object_assign_material(&bowser, &white,    2);
    object_assign_material(&bowser, &green,  1);
    object_assign_material(&bowser, &red,    3);
    add_object_to_scene(&settings,&bowser);

    while (run_renderer(&settings)) {
        rotate_y(&bowser,0.01f);
    }


    #elif 0
    

    // N64 Logo
    
    AsciiLUT *lut = &lut_xterm_256;
    float dithering = 0.1f;
    float specular = 0.5f;

    Material mtl_red        = ce_new_material_principled(lut, (Vec3){1.f,0.f,0.f},specular, dithering);
    Material mtl_yellow     = ce_new_material_principled(lut, C_ANSI_BRIGHT_YELLOW, specular, dithering);
    Material mtl_blue       = ce_new_material_principled(lut, (Vec3){0.2f, 0.2f, 1.f},specular, dithering);
    Material mtl_green      = ce_new_material_principled(lut, C_ANSI_BRIGHT_GREEN,  specular, dithering);
    Material mtl_white      = ce_new_material_principled(lut, C_ANSI_WHITE,         specular, dithering);


    Object o_n64   = ce_new_object_from_file("models/n64.obj");
    ce_object_assign_material(&o_n64, &mtl_yellow, 0);
    ce_object_assign_material(&o_n64, &mtl_green,  1);
    ce_object_assign_material(&o_n64, &mtl_blue,   2);
    ce_object_assign_material(&o_n64, &mtl_red,    3);
    ce_add_object_to_scene(settings,&o_n64);

    Object o_plane = ce_new_object_from_file("models/plane.obj");
    ce_object_assign_material(&o_plane, &mtl_white, 0);
    ce_add_object_to_scene(settings,&o_plane);

    Light light1 = {
        .pos=(Vec3)    { 8, 8, -5 }, 
        .color=(Color) { 1.f, 1.f, 1.f},
        .power=1.f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 7, -3 },
        .color=(Color) { 1.f, 0.5f, 0.f},
        .power=0.75f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);
    settings->render_shadows = true;

    while (ce_play(settings)) {
        ce_clear_canvas(settings);
        ce_render_frame(settings);
        ce_display_canvas(settings);       
        ce_rotate_y(&o_n64,0.01f);
    }
    


    #elif 1
    

    // Reflection Test
    
    

    float specular = 1.0f;

    Material mtl_white      = ce_new_material_principled(NULL, C_ANSI_WHITE,         specular, 0.0f);
    Material mtl_red        = ce_new_material_principled(NULL, C_ANSI_BRIGHT_RED,         specular, 0.0f);
    Material mtl_chrome      = ce_new_material_principled(NULL, C_BLACK,         0.0f, 0.0f);
    ce_principled_material_set_reflection(&mtl_chrome, 0.9f, 0.5f);
    ce_principled_material_set_ambient(&mtl_red, 0.1f);
    ce_principled_material_set_ambient(&mtl_white, 0.1f);

    Object o_sphere   = ce_new_object_from_file(settings, "models/sphere.obj");
    ce_object_assign_material(&o_sphere, &mtl_red, 0);
    ce_add_object_to_scene(settings,&o_sphere);
    ce_translate(&o_sphere, (Vec3){0,0.9f,0});

    Object o_sphere02 = o_sphere; 
    ce_object_assign_material(&o_sphere02, &mtl_white, 0);
    ce_add_object_to_scene(settings, &o_sphere02);
    ce_translate(&o_sphere02, (Vec3){-2,0,0});

    Object o_plane = ce_new_object_from_file(settings, "models/plane.obj");
    ce_object_assign_material(&o_plane, &mtl_chrome, 0);
    ce_add_object_to_scene(settings,&o_plane);
    
    int8_t tex_sky_clouds  = ce_import_texture(settings,"textures/sm64_clouds.bmp");
    Material mtl = ce_new_material_environment_texture(NULL, ENV_SPHERICAL, 1.0, 0.0f,tex_sky_clouds);
    settings->environment = &mtl;
    


    Light light1 = {
        .type=LIGHT_DIRECTIONAL,
        .pos=(Vec3)    { 8, 12, -5 }, 
        .color=(Color) { 1.f, 1.f, 1.f},
        .power=0.75f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 12, -3 },
        .color=(Color) { 1.f, 0.5f, 0.f},
        .power=0.75f
    };
    ce_add_light_to_scene(settings, &light1);
    //ce_add_light_to_scene(settings, &light2);
    settings->render_shadows = true;
    

    Material mtl_lut = ce_new_material_post_lut(lut_xterm_256_symbols, 0.25f);

    while (ce_play(settings)) {
        ce_clear_canvas(settings);
        ce_render_frame(settings);
        ce_draw_post_processing(settings, false, &mtl_lut);
        ce_display_canvas(settings);       
    }
    








    #elif 0
    // Black Hole

    Color dark_orange = {0.2, 0.15, 0.075};
    Color cyan        = {0.5,1,1};
    Material mtl = ce_new_material_post_black_hole(lut_xterm_256, dark_orange, cyan, 0.25f, 0.5f);
    settings->camera.pos.y = 8.0f;

    while (ce_play(settings))
    {
        ce_clear_canvas(settings);
        ce_draw_post_processing(settings, false, &mtl);
        ce_display_canvas(settings);
    }






    #elif 0
    // Metaballs

    Color colors[] = {C_RED, C_ANSI_BRIGHT_CYAN, C_YELLOW, C_GREEN_CYAN, C_GREEN, C_BLUE, C_ANSI_MAGENTA, C_ANSI_DARK_MAGENTA, C_RED};
    size_t num_colors = sizeof(colors) / sizeof(Color);
    float ambient = 0.6f;
    float diffuse = 0.5f;
    float specular = 0.75f;

    //Material mtl = ce_new_material_post_metaballs(lut_xterm_256, 12, colors, num_colors, ambient, diffuse, specular, 0.15f);
    Material mtl = ce_new_material_post_metaballs(lut_xterm_256_symbols, 12, colors,9, ambient, diffuse, specular, 0.25f);

    while (ce_play(settings))
    {
        ce_clear_canvas(settings);
        ce_draw_post_processing(settings, false, &mtl);
        ce_display_canvas(settings);
    }










    #elif 0


    Object o_tie   = ce_new_object_from_file(settings,"models/tie/tie.obj");
    float dithering = 0.35f;
    
    AsciiLUT *lut = lut_ansi_16_symbols;
    Material mtl_tie  = ce_new_material_principled(lut, C_TIE_BLUE, 1.0f, dithering);
    Material mtl_grey = ce_new_material_principled(lut, color_new_mono(0.0f), 0.0f, 0.05);
    Material mtl_red  = ce_new_material_principled(lut, C_ANSI_DARK_RED, 0.55f, dithering);

    ce_object_assign_material(&o_tie, &mtl_red, 2);
    ce_object_assign_material(&o_tie, &mtl_grey, 0);
    ce_object_assign_material(&o_tie, &mtl_tie, 1);
    ce_add_object_to_scene(settings,&o_tie);
    
    settings->background_color = C_ANSI_DARK_BLUE;
    //settings->render_shadows = true;

    Light light1 = {
        .pos=(Vec3)    { 8, 8, -5 }, 
        .color=C_ANSI_WHITE,
        .power=1.2f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 7, -3 },
        .color=C_ANSI_WHITE,
        .power=0.75f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);
    ce_rotate_y(&o_tie, 3.141f);

    while (ce_play(settings))
    {
        ce_object_manual_turntable(&o_tie);
        ce_clear_canvas(settings);
        Vec2 a = {w*0.1f, h*0.75f};
        Vec2 b = {w*0.6f, 30};
        Vec2 c = {40, 12};
        Vec2 verts[] = {a,b,c};
        
        Vec2 txt_pos = {10,8};
        ce_render_frame(settings);
        ce_draw_title(settings, C_ANSI_BLUE, C_WHITE, "  TIE Fighter - ANSI 16 Example ", " artstation.com/shellac ");

        //ce_draw_title_with_drop_shadow(settings, C_ANSI_BLUE, C_BLACK, " TIE Fighter - ANSI 16 ", " artstation.com/shellac ",C_BLACK, C_DARK_GREY);

        ce_draw_rectangle(settings, txt_pos.x+2, txt_pos.y+1, 63, 12,C_ANSI_BLACK);
        const char *text0 = 
            " > This demo uses the standard 16 color ANSI palette\n"
            "   and could be displayed on old terminals like MS-DOS.\n\n"
            " > The rendering is done in 16bit float, and the result\n"
            "   is then mapped to the target device palette\n\n"
            " > The optimal combinations of colors and ASCII symbol\n"
            "   can be pre-calculated and stored as a binary file";
        Vec2 anchor = ce_draw_info_window(settings, txt_pos.x,txt_pos.y, 63, 12, C_BLACK, C_WHITE,"","", text0, true);

        ce_display_canvas(settings);
    }
    
 #elif 0
    // Cassette

    Light light1 = {
        .pos=(Vec3)    { 8, 4, -5 }, 
        .color=(Color) { 1.f, 1.f, 1.f},
        .power=1.0f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 3, -3 },
        .color=(Color) { 1.f, 0.5f, 0.f},
        .power=0.5f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);

    Object statue  = ce_new_object_from_file(settings, "models/cassette/cassette.obj");
    //size_t t_statue = ce_import_texture(settings,"models/statue/statue.bmp");
    
    Color c_light_blue = {0.0f, 0.75f,1.0f};
    Color c_dark_blue = color_new_xterm_save_color(0,0,0.5);
    Color c_red       = color_to_xterm_save(C_PICO8_RED);
    Color c_pink      = color_to_xterm_save(C_PICO8_PINK);
    Color c_pale_yellow = color_new_xterm_save_color(1,1,0.5f);

    //Material mtl_blue = ce_new_material_gradient_ramp_blocks((Color[]){ c_dark_blue, c_light_blue}, 2, 0.25f, 0.025f);
    Material mtl_blue = ce_new_material_gradient_ramp_interpolated(lut_xterm_256, (Color[]){ c_dark_blue, c_light_blue}, 2, 0.25f, 0.025f);
    Material mtl_pink = ce_new_material_gradient_ramp_blocks((Color[]){ c_red, c_pink}, 2, 0.0f, 0.25f);
    Material mtl_white = ce_new_material_gradient_ramp_blocks((Color[]){ c_pale_yellow}, 1, 0.0f, 0.25f);
    Material mtl_black = ce_new_material_gradient_ramp_blocks((Color[]){ C_ANSI_BLACK, C_ANSI_WHITE}, 1, 0.0f, 0.25f);
    //ce_gradient_material_set_dither_pattern(&mtl_blue, DITHER_PATTERN_BAYER_4X4_BINARY);
    ce_gradient_material_set_dither_pattern(&mtl_pink, DITHER_PATTERN_BAYER_1X4_BINARY);

    
    mtl_blue.stencil = 1;
    mtl_pink.stencil = 1;
    mtl_white.stencil = 1;
    mtl_black.stencil = 1;
    ce_object_assign_material(&statue, &mtl_blue, 0);
    ce_object_assign_material(&statue, &mtl_pink, 1); // tape
    ce_object_assign_material(&statue, &mtl_black, 3);
    ce_object_assign_material(&statue, &mtl_pink, 4);
    ce_object_assign_material(&statue, &mtl_white, 2);
    ce_add_object_to_scene(settings,&statue);

    
    Material post = ce_new_material_post_horizontal_blur(lut_xterm_256, 16, 16, 0.1f, 0.1f);

    settings->background_color = c_dark_blue;
    settings->render_shadows = true;

    ce_rotate_y(&statue, 3.141);
    while (ce_play(settings)) 
    {
        ce_object_manual_turntable(&statue);
        ce_clear_canvas(settings);
        //ce_write_text(settings, 28,25, c_dark_blue, c_pale_yellow, "C:\\Mixtape\\run.exe");
        ce_render_frame(settings);
        ce_draw_post_processing(settings, true, &post);

        //ce_draw_title_with_drop_shadow(settings, c_pink, C_BLACK, " Cassette Tape - XTERM 256 ", " artstation.com/shellac ",C_BLACK, C_DARK_GREY);
        ce_draw_title(settings, c_pink, C_BLACK, " Cassette Tape - XTERM 256 ", " artstation.com/shellac ");
        ce_display_canvas(settings);
    }

    







    #elif 1
    // Statue

    Light light1 = {
        .pos=(Vec3)    { 8, 4, -5 }, 
        .color=(Color) { 1.f, 1.f, 1.f},
        .power=1.0f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 3, -3 },
        .color=(Color) { 1.f, 0.5f, 0.f},
        .power=0.5f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);

    Object statue  = ce_new_object_from_file(settings, "models/statue/statue.obj");
    size_t t_statue = ce_import_texture(settings,"models/statue/statue.bmp");
    

    Color c_orange = color_new_xterm_save_color(1.0f, 0.25f,0.0f);
    Material m_statue = ce_new_material_gradient_ramp_blocks((Color[]){ C_BLACK, c_orange, C_WHITE}, 3, 0.25f, 0.25f);
    ce_gradient_material_set_texture(&m_statue, t_statue);
    ce_gradient_material_set_brightness(&m_statue, 1.25f);

    
    m_statue.stencil = 1;
    ce_object_assign_material(&statue, &m_statue, 0);
    ce_add_object_to_scene(settings,&statue);
    size_t t_text = ce_import_texture(settings, "textures/title.bmp");
    TextureImage *x = settings->textures[t_text];

    
    // Glitchy background
    /*
    AsciiLUT *lut = calloc(1,sizeof(AsciiLUT));
    Vec3 colors[] = {C_DARK_GREY, C_BLACK, C_PICO8_LIGHT_PEACH, C_PICO8_BLUE, C_ORANGE, C_TIE_BLUE, C_PICO8_LAVENDER};
    ce_create_lookup_table(lut, colors, 7, CHARACTERS_BLOCK, 5);
    Material bg_mtl = {
        .shader=shader_screen_space_glitch,
        .parameters= &(ShaderParamsScreenSpaceGlitch){.lut = lut, .texture=tex_sky_lava, .colors=colors, .num_colors=7, .dither_amount=0.5f, .lut=&lut_ansi_16}
    };
    */

    Material sprite_text = {
        .shader=shader_sprite_principled,
        .parameters= &(ShaderParams_Sprite_Principled) {.opacity_tex_id = t_text, .color=C_GREEN, .bg_blend_method=BLEND_METHOD_COLOR_VIVID_LIGHT, .fg_blend_method=BLEND_METHOD_COLOR_VIVID_LIGHT}
    };
    
    Material post = ce_new_material_post_horizontal_blur(lut_xterm_256_symbols_vibrant, 16, 10, 0.1f, 0.25f);

    settings->background_color = C_GREY;
    settings->render_shadows = true;

    ce_rotate_y(&statue, 3.141);
    while (ce_play(settings)) 
    {
        ce_object_manual_turntable(&statue);
        ce_clear_canvas(settings);
        //ce_write_text(settings, 28,25, C_BLUE, C_WHITE, "CLEMesh m_torus  = import_obj(\"rubber.obj\");\nCLEObject torus = cle_new_object(&m_torus);\ncle_object_assign_material(&torus, &retro_two_tone, 0);\ncle_add_object_to_scene(settings,&torus);\nsettings->background_color = C_GREY;");
        ce_render_frame(settings);
        ce_draw_post_processing(settings, true, &post);
        //ce_write_text(settings, 25,35, C_WHITE, C_RED, "This should be in FROOOOOOOOOOONT\nof the rendered Geoemetry");
        //ce_draw_sprite(settings, 5, 5, 1*53, 1*13, &sprite_text);
        ce_display_canvas(settings);
    }
    //free_mesh(&m_torus);

    






    #elif 1
    // Deer

    Light light1 = {
        .pos=(Vec3)    { 8, 8, -5 }, 
        .color=(Color) { 1.f, 1.f, 1.f},
        .power=1.125f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 7, -3 },
        .color=(Color) { 1.f, 0.5f, 0.f},
        .power=0.75f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);

    
    Object deer  = ce_new_object_from_file(settings, "models/deer/deer.obj");
    size_t t_deer   = ce_import_texture(settings,"models/deer/deer.bmp");
    size_t t_antler = ce_import_texture(settings,"models/deer/antler.bmp");
    
    Color colors[] = {C_ANSI_BLACK, C_ANSI_BRIGHT_RED,C_WHITE};
    size_t num_colors = sizeof(colors)/sizeof(Color);
    
    Material m_deer = ce_new_material_gradient_ramp_interpolated(lut_ansi_16_symbols, colors, num_colors, 1.0, 0.45f);
    ce_gradient_material_set_brightness(&m_deer, 1.5f);
    ce_gradient_material_set_texture(&m_deer, t_deer);

    Material m_antler = ce_new_material_gradient_ramp_interpolated(lut_ansi_16_symbols, colors, num_colors, 1.0, 0.45f);
    ce_gradient_material_set_brightness(&m_antler, 1.5f);
    ce_gradient_material_set_texture(&m_antler, t_antler);
    
    m_deer.stencil = 1;
    m_antler.stencil = 1;
    ce_object_assign_material(&deer, &m_antler, 0);
    ce_object_assign_material(&deer, &m_antler, 1);
    ce_object_assign_material(&deer, &m_deer, 2);
    ce_add_object_to_scene(settings,&deer);


    Material post = ce_new_material_post_horizontal_glitches(lut_xterm_256_symbols_vibrant, 8, 25, 0.1f, 0.0f);

    settings->background_color = C_BLUE;
    settings->render_shadows = true;
    settings->camera.is_targeted = false;

    ce_rotate_y(&deer, 3.141);
    while (ce_play(settings)) 
    {
        ce_object_manual_turntable(&deer);
        ce_clear_canvas(settings);
        ce_render_frame(settings);
        ce_draw_post_processing(settings, true, &post);
        ce_display_canvas(settings);
    }













    #elif 0
    // glitchy skull dispersion
    
    Object skull   = ce_new_object_from_file(settings, "models/skull/skull.obj");
    size_t texture = ce_import_texture(settings,"models/skull/skull.bmp");
    
    float specular = 0.2f;
    float dithering = 0.05f;
    AsciiLUT *lut = lut_xterm_256_symbols;
    
    Color colors[] = {C_ANSI_BLACK, C_ANSI_BRIGHT_RED,C_WHITE};
    size_t num_colors = sizeof(colors)/sizeof(Color);
    
    Material mtl = ce_new_material_gradient_ramp_interpolated(lut_ansi_16_symbols, colors, num_colors, 0.3, 0.3);
    ce_gradient_material_set_brightness(&mtl, 1.75f);
    ce_gradient_material_set_texture(&mtl, texture);

    ce_object_assign_material(&skull, &mtl, 0);
    ce_add_object_to_scene(settings,&skull);
    
    Material blur_mtl = ce_new_material_post_dispersion(lut, 4, 35, 0.45);

    settings->render_shadows = true;
    settings->camera.focalDist = 2.0f;

    Light light1 = {
        .pos=(Vec3)    { 8, 8, -5 }, 
        .color=C_WHITE,
        .power=1.f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 7, -3 },
        .color=C_RED,
        .power=0.25f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);

    ce_rotate_y(&skull,-2.6495f);
    //return 0;

    while (ce_play(settings)) 
    {
        //ce_rotate_y(&skull,0.01f);
        ce_clear_canvas(settings);
        ce_render_frame(settings);
        ce_draw_post_processing(settings, true, &blur_mtl);

        Vec2 anchor = {5,60};
        Vec2 box_size = {70,10};
        Color text_bg = C_ANSI_BLUE;

        ce_draw_rectangle(settings, anchor.x+2, anchor.y+1, box_size.x, box_size.y,C_ANSI_DARK_BLUE);
        const char *text0 = 
            " > The render engine supports a plug-and-play 'interface' \n"
            "   for custom surface shader, 2D sprite shader, \n   and Post-Processing effects\n\n"
            " > The shaders can manipulate the background color, foreground color\n"
            "   and ascii character to created some nice glitchted cyberpunk visuals";
        ce_draw_info_window(settings, anchor.x, anchor.y, box_size.x, box_size.y, C_ANSI_BLUE, C_ANSI_WHITE," Scriptable Shaders ","", text0, true);

        //ce_write_text(settings, anchor.x + 1, anchor.y + 2, text_bg, C_WHITE, text0);
        ce_draw_object_info(settings, &skull, C_BLACK, C_RED, " Skull ");
        ce_draw_camera_info(settings,C_ANSI_BLACK, C_ANSI_BLUE);
        ce_display_canvas(settings);
    }

    #elif 0

    // glitchy elephant dispersion
    //
    Object elephant   = ce_new_object_from_file(settings, "models/elephant/elephant.obj");
    size_t texture = ce_import_texture(settings,"models/elephant/elephant.bmp");
    
    float specular = 0.25f;
    float dithering = 0.05f;
    //AsciiLUT *lut = lut_xterm_256_symbols;
    AsciiLUT *lut = lut_xterm_256_symbols_vibrant;
    
    Color c_green = color_new_xterm_save_color(0.7,1.0,0.1f);
    
    
    AsciiPixel tiles[] = {
        ce_new_ascii_pixel_single_byte(C_BLACK, C_ANSI_BLACK, " "),
        ce_new_ascii_pixel_single_byte(C_BLACK, C_ANSI_DARK_GREY, "_"),
        ce_new_ascii_pixel_single_byte(C_BLACK, C_ANSI_DARK_GREY, "*"),
        ce_new_ascii_pixel_single_byte(C_ANSI_DARK_GREY, c_green, "#"),
        ce_new_ascii_pixel_three_bytes(C_ANSI_DARK_GREY, c_green, PX_SQUARE),
        ce_new_ascii_pixel_single_byte(c_green, C_ANSI_WHITE, "_"),
        ce_new_ascii_pixel_three_bytes(c_green, C_WHITE, PX_SQUARE),
    };

    size_t num_tiles = sizeof(tiles)/sizeof(AsciiPixel); 

    Material mtl = ce_new_material_tiles(tiles, num_tiles, specular, dithering);
    ce_tile_material_set_texture(&mtl, texture);
    ce_tile_material_set_brightness(&mtl, 2.5f);
    ce_object_assign_material(&elephant, &mtl, 0);
    ce_add_object_to_scene(settings,&elephant);
   
    
    Material blur_mtl = ce_new_material_post_dispersion(lut, 4, 15, 0.0f);

    settings->render_shadows = true;
    settings->camera.focalDist = 2.0f;
    settings->background_color = color_new_xterm_save_color(0.73f, 0.73f, 0.73f);

    Light light1 = {
        .pos=(Vec3)    { 8, 8, -5 }, 
        .color=C_WHITE,
        .power=1.f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 7, -3 },
        .color=C_RED,
        .power=0.25f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);

    ce_rotate_y(&elephant,-0.922f);

    while (ce_play(settings)) 
    {
        ce_clear_canvas(settings);
        ce_render_frame(settings);

        Vec2 anchor = {5,60};
        Vec2 box_size = {70,10};
        Color text_bg = C_ANSI_BLUE;

        ce_draw_rectangle(settings, anchor.x+2, anchor.y+1, box_size.x, box_size.y, C_ANSI_DARK_GREY);
        const char *text = 
            " > The post process system uses an optional intermediate buffer \n"
            "   to avoid read after write scenarios.\n\n" 
            " > The post process can be drawn anytime. Even if it makes some\n"
            "   text in the info boxes unreadable ;).";
        ce_draw_info_window(settings, anchor.x, anchor.y, box_size.x, box_size.y, c_green, C_ANSI_BLACK," Scriptable Shaders ","", text, true);

        //ce_write_text(settings, anchor.x + 1, anchor.y + 2, text_bg, C_WHITE, text0);
        ce_draw_object_info(settings, &elephant, C_ANSI_DARK_GREY, c_green, "Object: Elephant ");
        ce_draw_camera_info(settings,C_ANSI_BLUE, C_ANSI_DARK_GREY);
        ce_draw_post_processing(settings, true, &blur_mtl);
        ce_draw_title(settings, C_ANSI_DARK_GREY, C_ANSI_BLUE, " Elephant Demo - XTERM 256 ", " artstation.com/shellac");
        ce_display_canvas(settings);
    }





    
    #elif 0

    // skull glitchy using BLUR
    //
    Object skull   = ce_new_object_from_file(settings, "models/skull/skull.obj");
    size_t texture = ce_import_texture(settings,"models/skull/skull.bmp");
   
    Color colors[] = {C_ANSI_BLACK, C_WHITE};
    size_t num_colors = sizeof(colors)/sizeof(Color);

    //Material mtl_skull = ce_new_material_gradient_ramp_blocks_textured(colors, num_colors, 0.3, 0.3,texture);
    //set_gradient_material_brightness(&mtl_skull, 1.2f);
    

    AsciiPixel tiles[] = {
        ce_new_ascii_pixel_single_byte(C_BLACK, C_ANSI_BLACK, " "),
        ce_new_ascii_pixel_single_byte(C_BLACK, C_ANSI_BLUE, "_"),
        ce_new_ascii_pixel_single_byte(C_BLACK, C_ANSI_BLUE, "i"),
        ce_new_ascii_pixel_single_byte(C_BLACK, C_ANSI_MAGENTA, "*"),
        //ce_new_ascii_pixel_single_byte(C_ANSI_DARK_RED, C_ANSI_MAGENTA, "*"),
        ce_new_ascii_pixel_single_byte(C_ANSI_BRIGHT_RED, C_ANSI_BLACK, "*"),
        ce_new_ascii_pixel_single_byte(C_ANSI_BLUE, C_ANSI_WHITE, "#"),
        ce_new_ascii_pixel_single_byte(C_ANSI_WHITE, C_ANSI_BLUE, "*"),
        ce_new_ascii_pixel_single_byte(C_ANSI_WHITE, C_ANSI_BRIGHT_GREEN, "_")
    };

    size_t num_tiles = sizeof(tiles)/sizeof(AsciiPixel); 
    
    float specular = 0.2f;
    float dithering = 0.05f;

    Material mtl = ce_new_material_tiles(tiles, num_tiles, 0.2f, dithering);
    ce_tile_material_set_texture(&mtl, texture);
    ce_tile_material_set_brightness(&mtl, 1.5f);
    //mtl.stencil = 1;

    ce_object_assign_material(&skull, &mtl, 0);
    ce_add_object_to_scene(settings,&skull);
    

    
    Material blur_mtl = ce_new_material_post_blur(lut_ansi_16_symbols, 4,1, 0.35);

    //settings->background_color = C_ANSI_DARK_MAGENTA;
    settings->render_shadows = true;
    settings->fog_enabled = false;
    settings->fog_start = 1.0f;
    settings->fog_end   = 10.0f;
    settings->camera.focalDist = 2.0f;

    Light light1 = {
        .pos=(Vec3)    { 8, 8, -5 }, 
        .color=C_WHITE,
        .power=1.f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 7, -3 },
        .color=C_RED,
        .power=0.25f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);

    

    ce_rotate_y(&skull,-2.6495f);
    while (ce_play(settings)) 
    {
        //ce_rotate_y(&skull,0.01f);
        ce_clear_canvas(settings);
        ce_render_frame(settings);
        ce_draw_post_processing(settings, true, &blur_mtl);
        ce_draw_camera_info(settings,C_ANSI_BLACK, C_ANSI_BLUE);
        


        Vec2 anchor = {5,60};
        Vec2 box_size = {70,10};
        Color text_bg = C_ANSI_BLUE;

        ce_draw_rectangle(settings, anchor.x+2, anchor.y+1, box_size.x, box_size.y,C_ANSI_DARK_BLUE);
        const char *text = 
            " > The render engine supports a plug-and-play 'interface' \n"
            "   for custom surface-, sprite- and post-process shaders\n\n"
            " > each shader can manipulate the background, foreground\n"
            "   and ASCII symbol.\n\n"
            " > Perfect to created some nice glitchy cyberpunk visuals.";
        ce_draw_info_window(settings, anchor.x, anchor.y, box_size.x, box_size.y, C_ANSI_BLUE, C_ANSI_WHITE," Scriptable Shaders ","", text, true);
        ce_draw_object_info(settings, &skull, C_BLACK, C_RED, " Skull ");
        
        //ce_draw_info_box(settings, C_ANSI_BLACK, C_PICO8_GREEN);
        ce_display_canvas(settings);
    }






















    #elif 0
    

    Object speeder  = ce_new_object_from_file("models/speeder/speeder.obj");
    size_t t_01 = ce_import_texture(settings,"models/speeder/speeder_01.bmp");
    size_t t_02 = ce_import_texture(settings,"models/speeder/speeder_02.bmp");
    size_t t_03 = ce_import_texture(settings,"models/speeder/speeder_03.bmp");

    float ambient=0.2f;
    float specular=1.5f;
   
    float dithering = 0.125;
    Material mtl_speeder_orange = ce_new_material_gradient_ramp_blocks_textured((Color[]){C_BLACK,C_ORANGE, C_YELLOW},3, specular, 0.3f, t_02);
    ShaderParams_Surface_GradientRamp *sh_orange = mtl_speeder_orange.parameters;
    sh_orange->ambient=ambient;

    Material mtl_booster = ce_new_material_gradient_ramp_blocks_textured((Color[]){C_DARK_TEAL,C_GREY},2, specular, dithering, t_01);
    ShaderParams_Surface_GradientRamp *sh_booster = mtl_booster.parameters;
    sh_booster->ambient=ambient;


    Material mtl_chrome = ce_new_material_chrome(&lut_xterm_256,C_LIGHTBLUE,0.2f, tex_sky_dark);




    
    Vec3 body_colors[] = {C_BLACK, C_GREY, C_LIGHTBLUE, C_WHITE};
    Material mtl_speeder_body = ce_new_material_gradient_ramp_blocks_textured(body_colors,sizeof(body_colors)/sizeof(Color), specular, dithering, t_03);
    ShaderParams_Surface_GradientRamp *sh_body = mtl_speeder_body.parameters;
    sh_body->ambient=ambient;
    

    Material mtl_env = ce_new_material_environment_texture(&lut_xterm_256, ENV_SPHERICAL, 0.5f, 0.05f, tex_sky_dark);
    

    ce_object_assign_material(&speeder, &mtl_speeder_body, 0);
    ce_object_assign_material(&speeder, &mtl_chrome, 1);
    ce_object_assign_material(&speeder, &mtl_speeder_orange, 2);
    ce_object_assign_material(&speeder, &mtl_booster, 3);

    ce_add_object_to_scene(settings,&speeder);
    ce_add_object_to_scene(settings,&sphere);
    //settings->camera.is_targeted = false;
    settings->background_color = C_DARKBLUE;
    settings->render_mode = RENDERMODE_DEBUG;
    settings->environment = &mtl_env;

    Light light1 = {
        .pos=(Vec3)    { 8, 12, -5 }, 
        .color=(Color) { 1.f, 1.f, 1.f},
        .power=1.0f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 7, -3 },
        .color=(Color) { 1.f, 1.f, 1.0f},
        .power=0.5f
    };
    ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);
    ce_translate(&speeder, (Vec3){0,1,0});

    while (ce_play(settings)) 
    {
        ce_clear_canvas(settings);
        ce_render_frame(settings);
        ce_draw_title(settings, C_BLACK, C_BLUE, "  CLI-Engine Spaceship Demo  ", " by Jonas Krein - artstation.com/shellac");
        ce_display_canvas(settings);
    }
    






    #elif 0


    Object lighthouse  = ce_new_object_from_file("models/lighthouse/lighthouse.obj");
    ce_add_object_to_scene(settings, &lighthouse);

    float dithering = 0.25f;
    float specular = 1.0f;
    

    #if 1 
    // color map to xterm 256
    AsciiLUT *lut = lut_ansi_16;
    //AsciiLUT *lut = lut_black_and_white;

    Material lh_water = ce_new_material_principled(lut, C_ANSI_BLUE, specular, dithering);
    Material lh_white = ce_new_material_principled(lut, C_WHITE,      specular, dithering);
    Material lh_red   = ce_new_material_principled(lut, C_RED,        specular, dithering);
    Material lh_grey  = ce_new_material_principled(lut, C_GREY,        specular, dithering);
    Material lh_rock  = ce_new_material_principled(lut, C_DARK_GREY, specular, dithering);

    #elif 0

    //Color colors[] = {C_ANSI_BLACK,  C_ANSI_DARK_GREY,  C_ANSI_WHITE};
    Color colors[] = {C_ANSI_BLACK, C_ANSI_DARK_CYAN,  C_ANSI_BRIGHT_CYAN, C_WHITE};
    uint8_t num_colors = 4;
    dithering = 0.05;

    Material lh_grey  =  ce_new_material_gradient_ramp_blocks(colors, num_colors, specular, dithering);
    Material lh_white =  ce_new_material_gradient_ramp_blocks(colors, num_colors, specular, dithering);
    Material lh_red   =  ce_new_material_gradient_ramp_blocks(colors, num_colors, specular, dithering);
    Material lh_water =  ce_new_material_gradient_ramp_blocks(colors, num_colors, specular, dithering);
    Material lh_rock  =  ce_new_material_gradient_ramp_blocks(colors, num_colors, specular, dithering);

    set_gradient_material_brightness(&lh_white, 1.2f);
    set_gradient_material_brightness(&lh_rock, 0.5f);
    set_gradient_material_brightness(&lh_red, 0.5f);
    set_gradient_material_brightness(&lh_water, 0.25f);

    #else
    // discrete ramps in ansi 16
    Vec3 body_colors[] = {C_BLACK, C_ANSI_LIGHT_GREY, C_WHITE};
    Material lh_grey =  ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK, C_DARK_GREY, C_LIGHT_GREY},3,specular, dithering);
    Material lh_white =  ce_new_material_gradient_ramp_blocks(body_colors,3,specular, dithering);
    Material lh_red   =  ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK, C_ANSI_DARK_RED, C_ANSI_BRIGHT_RED},3,specular, dithering);
    Material lh_water =  ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK, C_ANSI_DARK_BLUE, C_ANSI_BLUE, C_ANSI_BRIGHT_CYAN}, 4, specular, 0.25f);
    Material lh_rock  =  ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK, C_ANSI_DARK_GREY, C_ANSI_LIGHT_GREY},3,specular, dithering);
    #endif

    Material lamp     = ce_new_material_gradient_ramp_blocks((Color[]){C_YELLOW, C_WHITE}, 2, specular, dithering);
    lamp.stencil = 1;


    ce_object_assign_material(&lighthouse, &lh_white, 0); // white stripes
    ce_object_assign_material(&lighthouse, &lh_grey, 1);  // handrail
    ce_object_assign_material(&lighthouse, &lh_red, 2);  // red stripes
    ce_object_assign_material(&lighthouse, &lh_white, 3);  // Balcony
    ce_object_assign_material(&lighthouse, &lh_water, 4);  // window
    ce_object_assign_material(&lighthouse, &lh_white, 5);  // white struts around lamp
    ce_object_assign_material(&lighthouse, &lamp, 6);  // lamp
    ce_object_assign_material(&lighthouse, &lh_white, 7);  // house wall
    ce_object_assign_material(&lighthouse, &lh_red, 8);  // house roof
    ce_object_assign_material(&lighthouse, &lh_grey, 9);  // house door
    ce_object_assign_material(&lighthouse, &lh_rock, 10);  // island
    ce_object_assign_material(&lighthouse, &lh_water, 11);  // water

    Light light1 = {
        .pos=(Vec3)    { 8, 8, -5 }, 
        .color=C_ORANGE,
        .power=0.5f,
    };
    Light light2 = {
        .type= LIGHT_DIRECTIONAL,
        .pos=(Vec3)    { -10, 7, -3 },
        .color=C_WHITE,
        .power=0.75f
    };
    //ce_add_light_to_scene(settings, &light1);
    ce_add_light_to_scene(settings, &light2);
    settings->render_shadows   = true;
    settings->background_color = C_DARKBLUE;
    settings->background_color = C_BLACK;

    settings->fog_enabled = true;
    settings->fog_start = 8.0f;
    settings->fog_end   = 20.0f;
    

    Material bloom_mtl = ce_new_material_post_bloom(lut, dithering);
   

    while (ce_play(settings)) 
    {
        ce_rotate_y(&lighthouse,0.01f);
        ce_clear_canvas(settings);
        ce_render_frame(settings);
        ce_draw_full_screen(settings, &bloom_mtl);
        ce_draw_title(settings, C_ANSI_BLACK, C_ANSI_WHITE, "  CLI-Engine - Lighthouse Demo  ", " artstation.com/shellac ");

        Vec2 anchor = {120,10};
        Vec2 box_size = {70,10};
        Color text_bg = C_ANSI_BLUE;
        Color text_shadoe = C_ANSI_DARK_BLUE;

        ce_draw_rectangle(settings, anchor.x+2, anchor.y+1, box_size.x, box_size.y,C_ANSI_DARK_BLUE);
        ce_draw_info_window_frame(settings, anchor.x, anchor.y, box_size.x, box_size.y, C_ANSI_BLUE, C_ANSI_WHITE," Scriptable Post Processing ","", true);
        const char *text0 = 
            " > Example of a basic (unoptimized) post-process bloom effect.\n\n"
            " > The light material is set to write directly to a stencil buffer,\n   which the full-screen bloom shader can use to cleanly\n   isolate the light source.";
        ce_write_text(settings, anchor.x + 1, anchor.y + 2, text_bg, C_WHITE, text0);
        
        //char txt[128];
        //snprintf(txt,128,"Focal Dist: %f\nResolution: %d x %d", settings->camera.focalDist, settings->render_width, settings->render_height);
        //ce_write_text(settings, 7,7, settings->background_color, C_PICO8_RED, txt);
        //ce_draw_info_box(settings, C_ANSI_BLACK, C_PICO8_GREEN);
        ce_display_canvas(settings);
    }
    //free_mesh(&city->mesh);





    #elif 0

        Object western  = ce_new_object_from_file(settings, "models/western_diorama/western.obj");
        ce_add_object_to_scene(settings, &western);

        float dithering = 0.25f;
        float specular = 1.0f;
        

        // color map to xterm 256
        AsciiLUT *lut = lut_ansi_16;

        
        Color c_sand = color_new_xterm_save_color(1.0f, 0.75f, 0.5f);
        Color c_red = color_new_xterm_save_color(0.75f, 0.25f, 0.25f);
        Color c_light_red = color_new_xterm_save_color(1.0f, 0.35f, 0.35f);
        Color c_saloon = color_new_xterm_save_color(1.0f, 1.0f, 0.85f);
        Color c_tan = color_new_xterm_save_color(1.0f, 0.85f, 0.25f);
        Color c_orange       = color_new_xterm_save_color(0.9f, 0.5f, 0.15f);
        Color c_wood            = color_new_xterm_save_color(0.6f, 0.4f, 0.1f);
        Color c_dark_wood       = color_new_xterm_save_color(0.2f, 0.1f, 0.15f);
        Color c_light_blue = color_new_xterm_save_color(0.5, 0.75f, 1.0f);
        Color c_red_orange = color_new_xterm_save_color(1.0f, 0.25f, 0.0f);


        //const char *simpleChars[] = {BLK_BLANK, PX_HASH, BLK_SOLID};

        #if 0
        const Color colors[] = {
            c_sand,
            c_red,
            C_WHITE,
            C_YELLOW,
            C_LIGHT_GREY,
            C_DARK_GREY,
            c_red_orange,
            c_light_blue,
            c_wood,
            c_tan,
            C_ANSI_BLUE,
            C_BLACK,
            C_ANSI_BRIGHT_CYAN
        }; 
        lut = lut_black_and_white;

        Material lh_water = ce_new_material_principled(lut, C_PICO8_BLUE, specular, dithering);
        Material lh_white = ce_new_material_principled(lut, C_WHITE,      specular, dithering);
        Material lh_tarp  = ce_new_material_principled(lut, C_WHITE,      0.25f, 0.35f);
        Material lh_red   = ce_new_material_principled(lut, C_RED,        specular, dithering);
        Material lh_grey  = ce_new_material_principled(lut, C_GREY,        specular, dithering);
        Material lh_rock  = ce_new_material_principled(lut, C_LIGHT_GREY, specular, dithering);
        Material lh_wood = ce_new_material_principled(lut, c_wood, specular, dithering);
        Material lh_dark_wood = ce_new_material_principled(lut, c_dark_wood, specular, dithering);
        Material lh_saloon = ce_new_material_principled(lut, c_saloon, specular, dithering);
        Material lh_tan    = ce_new_material_principled(lut, c_tan, specular, dithering);
        settings->background_color = c_light_blue;

        #elif 0
        // discrete ramps in ansi 16
        Material lh_white  = ce_new_material_gradient_ramp_blocks((Color[]){c_dark_wood, c_sand, C_WHITE,C_WHITE}, 4, specular,dithering);
        Material lh_grey  = ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK,C_ANSI_DARK_GREY, C_ANSI_LIGHT_GREY}, 3, specular,dithering);

        Material lh_red   =  ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK, C_ANSI_DARK_RED, C_ANSI_BRIGHT_RED, c_light_red}, 4,specular, dithering);
        Material lh_brown  =  ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK, C_ANSI_DARK_YELLOW, C_ANSI_LIGHT_GREY}, 3,specular, dithering);
        Material lh_wood   =  ce_new_material_gradient_ramp_blocks((Color[]){c_dark_wood, c_wood, c_sand }, 3, specular, dithering);
        Material lh_dark_wood   =  ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK, c_dark_wood, c_wood, }, 3, specular, dithering);
        Material lh_tan         =  ce_new_material_gradient_ramp_blocks((Color[]){c_dark_wood, c_wood, c_tan }, 3, specular, dithering);
        Material lh_rock  = lh_tan;
        
        #else
        //Color bw[] = {C_ANSI_BLACK, C_DARK_GREY,C_LIGHT_GREY, C_ANSI_WHITE};
        Color bw[] = {C_ANSI_BLACK,  C_ANSI_WHITE};
        size_t num_colors = sizeof(bw)/sizeof(Color);

        Material lh_white        = ce_new_material_gradient_ramp_blocks(bw, num_colors, specular,dithering);
        Material lh_wood         = ce_new_material_gradient_ramp_blocks(bw, num_colors, specular,dithering);
        Material lh_red          = ce_new_material_gradient_ramp_blocks(bw, num_colors, specular,dithering);
        Material lh_grey         = ce_new_material_gradient_ramp_blocks(bw, num_colors, specular,dithering);
        Material lh_dark_wood    = ce_new_material_gradient_ramp_blocks(bw, num_colors, specular,dithering);
        Material lh_tan          = ce_new_material_gradient_ramp_blocks(bw, num_colors, specular,dithering);
        Material lh_rock         = ce_new_material_gradient_ramp_blocks(bw, num_colors, specular,dithering);
        

        ce_gradient_material_set_brightness(&lh_grey, 0.75);
        ce_gradient_material_set_brightness(&lh_dark_wood, 0.35);
        ce_gradient_material_set_brightness(&lh_wood, 1.0f);
        ce_gradient_material_set_brightness(&lh_tan, 0.75);
        
        #endif

        Material lamp     = ce_new_material_gradient_ramp_blocks((Color[]){C_ANSI_BLACK, C_ANSI_WHITE},1,1.0f,0.25f);


        ce_object_assign_material(&western, &lh_tan, 0); // floor
        ce_object_assign_material(&western, &lh_red, 1);   // house right first
        ce_object_assign_material(&western, &lh_grey, 2);  // Wagon Wheels
        ce_object_assign_material(&western, &lh_dark_wood, 3);     // Roofs Balconies and window frames
        ce_object_assign_material(&western, &lh_wood, 4);  // pole
        ce_object_assign_material(&western, &lh_white, 5);  // House second right
        ce_object_assign_material(&western, &lamp, 6);  // Lit windows
        ce_object_assign_material(&western, &lh_white, 7);  // church 
        ce_object_assign_material(&western, &lh_red, 8);  // Single House
        ce_object_assign_material(&western, &lh_wood, 9);  // Church Cross + house door
        ce_object_assign_material(&western, &lh_dark_wood, 10);  // isolated wheel
        ce_object_assign_material(&western, &lh_rock, 11);  // pebbles
        ce_object_assign_material(&western, &lh_dark_wood, 12);  // WAgon body
        ce_object_assign_material(&western, &lh_white, 13);  // wagon tarp
        ce_object_assign_material(&western, &lh_rock, 14);  // more pebbles
        ce_object_assign_material(&western, &lh_white, 15);  // pebbles
        ce_object_assign_material(&western, &lh_white, 16);  // plant

        Light light1 = {
            .pos=(Vec3)    { 0, 3, -8 }, 
            .color=C_WHITE,
            .power=0.5f,
        };
        Light light2 = {
            .type= LIGHT_DIRECTIONAL,
            .pos=(Vec3)    { 1, 1, -0.5 },
            .color=C_WHITE,
            .power=1.0f
        };
        //ce_add_light_to_scene(settings, &light1);
        ce_add_light_to_scene(settings, &light2);
        settings->render_shadows = true;

        settings->fog_enabled = true;
        settings->fog_start = 15.0f;
        settings->fog_end   = 25.0f;
        
        settings->camera.is_targeted = false;
        settings->camera.focalDist = 1.7f;
        settings->camera.pos = (Vec3){11.51f, -0.125f, -12.38f};
        settings->camera.pitch = -0.075f;
        settings->camera.yaw   = -0.65f;
        camera_update_transform(&settings->camera);
        
        settings->background_color = C_ANSI_BLACK;
        
        size_t t_text = ce_import_texture(settings, "models/western_diorama/western_text.bmp");
        size_t sprite_w = settings->textures[t_text]->width;
        size_t sprite_h = settings->textures[t_text]->height;

        Material sprite_text = {
            .shader=shader_sprite_principled,
            .parameters= &(ShaderParams_Sprite_Principled) {.opacity_tex_id = t_text, .color=C_ANSI_BRIGHT_YELLOW, .bg_blend_method=BLEND_METHOD_COLOR_OVERWRITE, .fg_blend_method=BLEND_METHOD_COLOR_OVERWRITE}
        };
        

        ce_rotate_y(&western,-3.141f/2.0f);
        while (ce_play(settings)) 
        {
            //ce_rotate_y(&western,0.01f);
            ce_clear_canvas(settings);
            ce_render_frame(settings);
            ce_draw_title(settings, C_ANSI_BLACK, C_WHITE, " Western RPG Mockup ", " artstation.com/shellac ");
            
            Vec2 anchor = ce_draw_info_window(settings, 150,64,50,10, C_BLACK, C_WHITE,"","", NULL, true);

            char *text0 = "You come across a seemingly deserted town.\n\n * look around";
            ce_write_text(settings, anchor.x + 3, anchor.y + 2, C_BLACK, C_WHITE, text0);
            char *text1 = " * leave       ";
            ce_write_text(settings, anchor.x + 3, anchor.y + 5, C_WHITE, C_BLACK, text1);
            
            //ce_draw_sprite(settings, 2, 65, 2*sprite_w, sprite_h, &sprite_text);
            
            ce_draw_camera_info(settings, C_BLACK, C_WHITE);
            ce_display_canvas(settings);
        }
        //free_mesh(&city->mesh);
    

    #elif 0
    
    // City
    // ////////////////////////////////////////
    Object city  = ce_new_object_from_file(settings,"models/city/city.obj");
    /*
    Object building_00  = ce_new_object_from_file("models/city/building_00.obj");
    Object building_01  = ce_new_object_from_file("models/city/building_01.obj");
    Object building_02  = ce_new_object_from_file("models/city/building_02.obj");
    Object building_03  = ce_new_object_from_file("models/city/building_03.obj");

    Object building_04  = ce_new_object_from_file("models/city/building_04.obj");
    Object building_05  = ce_new_object_from_file("models/city/building_05.obj");
    Object building_06  = ce_new_object_from_file("models/city/building_06.obj");
    Object building_07  = ce_new_object_from_file("models/city/building_07.obj");

    Object building_08  = ce_new_object_from_file("models/city/building_08.obj");
    Object building_09  = ce_new_object_from_file("models/city/building_09.obj");
    Object building_10  = ce_new_object_from_file("models/city/building_10.obj");
    Object building_11  = ce_new_object_from_file("models/city/building_11.obj");

    Object building_12  = ce_new_object_from_file("models/city/building_12.obj");
    Object building_13  = ce_new_object_from_file("models/city/building_13.obj");
    Object building_14  = ce_new_object_from_file("models/city/building_14.obj");
    Object building_15  = ce_new_object_from_file("models/city/building_15.obj");

    Object building_16  = ce_new_object_from_file("models/city/building_16.obj");
    Object building_17  = ce_new_object_from_file("models/city/building_17.obj");
    Object building_18  = ce_new_object_from_file("models/city/building_18.obj");
    Object building_19  = ce_new_object_from_file("models/city/building_19.obj");
    Object building_20  = ce_new_object_from_file("models/city/building_20.obj");
    */
    
    size_t t_city = ce_import_texture(settings,"models/city/material_baseColor.bmp");
    //import_texture(settings,"models/city/ground_baseColor.bmp",   11);
    
    float dithering = 0.45f;
    float ambient = 0.2f;
    Color background = color_new_xterm_save_color(0.0f, 0.0f, 0.1f);
    Color c_blue = color_to_xterm_save(C_PICO8_BLUE);
    Material mtl_city_red = ce_new_material_gradient_ramp_blocks((Color[]){background, C_ANSI_BRIGHT_RED}, 2, 1.0f, dithering);

    Material mtl_city_blue = ce_new_material_gradient_ramp_blocks((Color[]){background, c_blue}, 2, 1.0f, dithering);
    ce_gradient_material_set_texture(&mtl_city_blue, t_city);
    ce_gradient_material_set_ambient(&mtl_city_blue, ambient);
    // Material mtl_city_blue = ce_new_material_retro_textured(C_PICO8_BLUE,     2.0f, dithering, 10);
    //((ShaderParametersRetro*)mtl_city_blue.parameters)-> brightness = 2.0f;
    //((ShaderParametersRetro*)mtl_city_blue.parameters)-> ambient = ambient;

    Material mtl_city_yellow = ce_new_material_gradient_ramp_blocks((Color[]){background, C_ANSI_BRIGHT_YELLOW}, 2, 1.0f, dithering);
    ce_gradient_material_set_texture(&mtl_city_yellow, t_city);
    ce_gradient_material_set_ambient(&mtl_city_yellow, ambient);
    //Material mtl_city_yellow = ce_new_material_retro_textured(C_PICO8_YELLOW,         1.0f, dithering, 10);
    //((ShaderParametersRetro*)mtl_city_yellow.parameters)-> brightness = 2.0f;
    //((ShaderParametersRetro*)mtl_city_yellow.parameters)-> ambient = ambient;

    //Material mtl_city_grey = ce_new_material_retro_textured(C_PICO8_LAVENDER,         1.0f, dithering, 10);
    //((ShaderParametersRetro*)mtl_city_grey.parameters)-> brightness = 2.0f;
    //((ShaderParametersRetro*)mtl_city_grey.parameters)-> ambient = 0.2f;

/*
    ce_object_assign_material(&building_00, &mtl_city_blue,    0);
    ce_object_assign_material(&building_01, &mtl_city_yellow, 0);
    ce_object_assign_material(&building_02, &mtl_city_blue,   0);
    ce_object_assign_material(&building_03, &mtl_city_red,    0);

    ce_object_assign_material(&building_04, &mtl_city_blue,   0);
    ce_object_assign_material(&building_05, &mtl_city_yellow, 0);
    ce_object_assign_material(&building_06, &mtl_city_blue,   0);
    ce_object_assign_material(&building_07, &mtl_city_blue,   0);

    ce_object_assign_material(&building_08, &mtl_city_blue,    0);
    ce_object_assign_material(&building_09, &mtl_city_yellow, 0);
    ce_object_assign_material(&building_10, &mtl_city_blue,   0);
    ce_object_assign_material(&building_11, &mtl_city_red,   0);

    ce_object_assign_material(&building_12, &mtl_city_yellow, 0);
    ce_object_assign_material(&building_13, &mtl_city_blue,   0);
    ce_object_assign_material(&building_14, &mtl_city_red,   0);
    ce_object_assign_material(&building_15, &mtl_city_blue, 0);

    ce_object_assign_material(&building_16, &mtl_city_blue,    0);
    ce_object_assign_material(&building_17, &mtl_city_red,    0);
    ce_object_assign_material(&building_18, &mtl_city_blue,   0);
    ce_object_assign_material(&building_19, &mtl_city_yellow, 0);
    ce_object_assign_material(&building_20, &mtl_city_blue,   0);
  */  



    #if 0
    Object buildings[] = {
        building_00,
        building_01,
        building_02,
        building_03,
        building_04,
        building_05,
        building_06,
        building_07,
        building_08,
        building_09,
        building_10,
        building_11,
        building_12,
        building_13,
        building_14,
        building_15,
        building_16,
        building_17,
        building_18,
        building_19,
        building_20,
    };

    CE_PointCloud grid = ce_make_point_grid(8,8,1.f,1.f);
    ce_scatter_objects(settings, &grid, buildings, 21);
    free(grid.points);
    #else




    ce_object_assign_material(&city, &mtl_city_red, 0);
    ce_object_assign_material(&city, &mtl_city_blue, 1);
    ce_object_assign_material(&city, &mtl_city_yellow, 2);
    ce_object_assign_material(&city, &mtl_city_blue, 3);
    ce_object_assign_material(&city, &mtl_city_yellow, 4);
    ce_add_object_to_scene(settings,&city);

    //ce_object_assign_material(&plane, &mtl_city_grey, 0);
    //ce_add_object_to_scene(settings,&plane);
    #endif


    //settings->camera.is_targeted = false;
    settings->background_color = background;
    settings->fog_enabled = true;
    settings->fog_start = 15.0f;
    settings->fog_end   = 22.0f;
    settings->render_shadows = true;
    settings->camera.focalDist = 3.2f;
    
    settings->camera.pos = (Vec3){7.89f, 10.89, -13.21f};
    settings->camera.pitch = 0.0f;
    settings->camera.pitch = 0.0f;
    camera_update_transform(&settings->camera);


    Light light1 = {
        .type=LIGHT_DIRECTIONAL,
        .pos=(Vec3)    { 8, 8, -5 }, 
        .color=C_WHITE,
        .power=1.f
    };
    Light light2 = {
        .pos=(Vec3)    { -10, 7, -3 },
        .color=C_WHITE,
        .power=0.0f
    };
    ce_add_light_to_scene(settings, &light1);
    //ce_add_light_to_scene(settings, &light2);

    ce_rotate_y(&city,90.f);

    while (ce_play(settings)) 
    {
        ce_rotate_y(&city,0.01f);
        ce_clear_canvas(settings);
        ce_render_frame(settings);
        ce_draw_title(settings, C_ANSI_BLACK, C_ANSI_BLUE, "  CLI-Engine City Demo  ", " artstation.com/shellac ");
        ce_draw_camera_info(settings, C_BLACK, C_WHITE);
        ce_draw_object_info(settings, &city, C_BLACK, C_WHITE, " City ");
        ce_display_canvas(settings);
    }
    //free_mesh(&city->mesh);
    
    #endif




    free(lut_xterm_256);
    free(lut_xterm_256_symbols);
    free(lut_xterm_256_symbols_vibrant);
    free(lut_ansi_16);
    free(lut_ansi_16_symbols);

    printf("exiting with no errors");
    return 0;
}
