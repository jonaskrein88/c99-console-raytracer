#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>

#include "shaders.h"
#include "vector.h"
#include "renderer.h"
#include "lookup.h"
#include "shader_utils.h"






















static inline void evaluate_light(Light *light, Vec3 fragPos, float *out_falloff, Vec3 *out_lightDir)
{
    switch(light->type)
    {
        case LIGHT_POINT:
            *out_lightDir = vec3_normalize( vec3_sub(light->pos, fragPos) );
            float dist = vec3_distance(light->pos, fragPos) / 10.0;
            *out_falloff = 1.0 / fmaxf(dist * dist, 1.0);
            return;

        case LIGHT_DIRECTIONAL:
            *out_lightDir = vec3_normalize( light->pos);
            *out_falloff = 1.0;
            return;
    }
}




static Color bsdf_principled(RenderSettings *renderSettings, FragmentData *frag, Color baseColor, float ambient, float spec_amount, float spec_sharpness, float rim_light, bool use_texture, uint8_t tex_idx)
{
    float fresnel = calculate_fresnel(frag->normal, frag->view_direction, 0.4f);
    spec_sharpness = powf(255.0,spec_sharpness);

    if (use_texture) {
        baseColor = sample_texture(renderSettings, tex_idx, frag->uv.x, frag->uv.y);
    }

    // init accum with the ambient as a base 
	Color color_accum = vec3_mul(baseColor,ambient);

	for (size_t i=0;i<renderSettings->scene.numLights; i++)
	{
        Light *light = renderSettings->scene.lights[i];

		Vec3 lightDir;
        float falloff;
        evaluate_light(light,frag->pos, &falloff, &lightDir);


        // bail shadow ray and such if falloff is to low anyhow
        if (falloff<0.01) continue;
        if (renderSettings->render_shadows==true)
        {
            bool occlusion = calculate_shadow(renderSettings,frag->pos, lightDir);
            if (occlusion) continue;
        }

		float diffuseAmount = fmaxf(0.f,vec3_dot(lightDir,frag->normal));
		diffuseAmount = fminf(1.f, diffuseAmount * falloff );

        Vec3 lightColor = vec3_mul(light->color, light->power);
        Vec3 finalDiffuse = vec3_mul(baseColor, diffuseAmount);
        finalDiffuse = vec3_mul_vec3(finalDiffuse,lightColor);
        
        // specular
        Vec3 viewDir    = vec3_normalize( vec3_sub( renderSettings->camera.pos, frag->pos));
        Vec3 halfwayDir = vec3_normalize( vec3_add( lightDir, viewDir));
        float spec = pow(fmax(vec3_dot(frag->normal, halfwayDir), 0.0), spec_sharpness);
        spec *= falloff * spec_amount;
        Color s = vec3_mul(lightColor, spec*fresnel);

        
        Vec3 out = vec3_add(finalDiffuse, s);
        color_accum = vec3_add(color_accum, out);
        
	}






    // add a bit of fresnel
    color_accum = vec3_add(vec3_mul(C_WHITE, rim_light * fresnel),  color_accum);

    // debug depth buffer
    //color_accum = vec3_mul(C_WHITE, frag->depth);
    return vec3_saturate(color_accum);
}








static float bsdf_simple_mono(RenderSettings *renderSettings, FragmentData *frag, float baseBrightness, float ambient, float spec_amount, float spec_sharpness, bool use_texture, uint8_t tex_idx)
{
    float fresnel = calculate_fresnel(frag->normal, frag->view_direction, 0.4f);
    spec_sharpness = powf(255.0,spec_sharpness);

    if (use_texture) {
        Color tex = sample_texture_bilinear(renderSettings, tex_idx, frag->uv.x, frag->uv.y);
        baseBrightness *= luma(tex);
    }
   
    // init accum with the ambient as a base 
    float brightness_accum = baseBrightness*ambient;

	for (size_t i=0;i<renderSettings->scene.numLights; i++)
	{
        Light *light = renderSettings->scene.lights[i];
        
		Vec3 lightDir;
        float falloff;
        evaluate_light(light,frag->pos, &falloff, &lightDir);

        // bail if it's too dark anyhow
        if (falloff<0.01) continue;

        
        // cast shadow ray
        if (renderSettings->render_shadows==true) {
            bool occlusion = calculate_shadow(renderSettings,frag->pos, lightDir);
		    if (occlusion) continue;
        }
        

		float diffuseAmount = fmaxf(0.f,vec3_dot(lightDir,frag->normal));
		diffuseAmount = fmin(1.f, diffuseAmount * falloff * light->power * baseBrightness);
        float out = diffuseAmount; 

        // Specular
        if (spec_amount > 0.0f){
            Vec3 viewDir    = vec3_normalize( vec3_sub( renderSettings->camera.pos, frag->pos));
            Vec3 halfwayDir = vec3_normalize( vec3_add( lightDir, viewDir));
            float spec = pow(fmax(vec3_dot(frag->normal, halfwayDir), 0.0), spec_sharpness);
            spec *= falloff * light->power;
            //out = lerpf(diffuseAmount,spec,fresnel*specularAmount); 
            out += spec * fresnel * spec_amount; 
        }
        
        brightness_accum += out;
        //spec *= falloff * specularAmount * fresnel;
        //brightness_accum += spec*ambient;
	}
    return brightness_accum;
}



















// the main shader for the retro two-tone or gradient look
void shader_surface_gradient_ramp(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Surface_GradientRamp *params = payload;
    
    float surface = bsdf_simple_mono(renderSettings, frag,
        params->brightness,
        params->ambient,
        params->spec_amount,
        params->spec_sharpness,
        params->use_texture,
        params->tex_id
    );

    float fog = calculate_fog(renderSettings, frag);
    surface *= fog;

    // debug - just the colors with interpolation
    if (renderSettings->render_mode == RENDERMODE_DEBUG){
        Color a,b;
        float t = gradient_map(surface, params->colors, params->num_colors, &a, &b);
        Color rgb = vec3_lerp(a,b,t);
        _ascii_shader_full(rgb,out_pixel);
        return;
    }
    
    float x = params->dither_amount;

    // discrete steps
    // - only the target colors are allowed
    if (params->discrete_steps)
    {
        surface = dither_float(surface, frag->x_coord, frag->y_coord, x, params->dither_pattern);
        surface = clamp01(surface);
        Color a,b;
        float t = gradient_map(surface, params->colors, params->num_colors, &a, &b);
        //if      (t <= 1.0f / params->num_colors)   _ascii_shader_from_symbol_list(b, b, params->symbols, params->num_symbols, t, out_pixel);
        //else if (t > 1.0f - 1.0f/params->num_colors) _ascii_shader_from_symbol_list(a, a, params->symbols, params->num_symbols, t, out_pixel);
        //else 
        _ascii_shader_from_symbol_list(b, a, params->symbols, params->num_symbols, t, out_pixel);
    }
    
    // interpolated colors
    // will create inbetween colors that will be quantized by palette LUT if present
    else
    {
        Color a,b;
        float t = gradient_map(surface, params->colors, params->num_colors, &a, &b);
        Color rgb = vec3_lerp(a,b,t);
        if(luma(rgb)>0.01f) rgb = dither(rgb, frag->x_coord, frag->y_coord, x);
        if (params->lut != NULL){
           _ascii_shader_from_lut(rgb, params->lut, out_pixel); 
           return;
        }
        else{
            _ascii_shader_full(rgb,out_pixel);
            return;
        }
    }
}


static inline Color color_random_from_palette(const Color **palette, size_t palette_size, size_t seed){
    uint8_t index = (int)(seed * 3) % palette_size;
    return *palette[index];
}


// surface shader that will only place "prefab" characters based on input brightness
void shader_surface_tiles(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Surface_Tiles *params = payload;
    
    float surface = bsdf_simple_mono(renderSettings, frag,
        params->brightness,
        params->ambient,
        params->spec_amount,
        params->spec_sharpness,
        params->use_texture,
        params->tex_id
    );

    float fog = calculate_fog(renderSettings, frag);
    surface *= fog;

    if (renderSettings->render_mode == RENDERMODE_DEBUG){
        _ascii_shader_full(color_new_mono(surface),out_pixel);
        return;
    }
    
    float x = params->dither_amount;
    surface = dither_float(surface, frag->x_coord, frag->y_coord, x, params->dither_pattern);
    //surface = make_glitchy_dithering(c_new_mono(surface), frag->x_coord, frag->y_coord, 0.25).x;

    surface = clamp01(surface);
    int index = (int)(surface * (params->num_tiles-0.01f));
    AsciiPixel tile =  params->tiles[index];

    Vec2 r = hash22((Vec2){frag->x_coord, frag->y_coord});
    
    if (r.y < 0.1f)
    {
        size_t i = r.x * 95; // 95
        char random_ascii = 32 + i; // 32
        tile.bytes[0] = random_ascii;
        tile.bytes[1] = 0;
        tile.bytes[2] = 0;
    }

    if (r.y < 0.01f)
    {
        if (luma(tile.bg)>0.01)
        {
            uint8_t bg_index = (int)(r.y *315) % 16;
            uint8_t fg_index = (int)(r.y *127+75) % 16;
            tile.bg = *C_ANSI_PALETTE[bg_index];
            tile.fg = *C_ANSI_PALETTE[fg_index];
        }
    }
    
    memcpy(out_pixel, &tile, sizeof(AsciiPixel));
    
}













// Main shader for "PBR"
// - will take a lut and apply it 
// - or output the raw color in both BG and FG
void shader_surface_principled(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    if (payload == NULL)
    {
        _ascii_shader_blocks((Vec3){1.0,0.0,1.0}, renderSettings->background_color, 1.f, out_pixel);
    }
    
    ShaderParams_Surface_Principled *params = (ShaderParams_Surface_Principled*)payload;
    
    Color surface = bsdf_principled(renderSettings,frag, 
        params->color,
        params->ambient,
        params->spec_amount,
        params->spec_sharpness,
        params->rim_light_amount,
        params->use_texture,
        params->tex_id);

    if (params->reflection_amount > 0.0)
    {
        float fresnel = calculate_fresnel(frag->normal, frag->view_direction, params->reflection_fresnel);
        HitResult hit_result = {0};
        AsciiPixel p = {0};
        cast_reflection_ray(renderSettings, frag->pos, frag->normal, frag->view_direction, &hit_result);
        process_ray(renderSettings,frag->x_coord, frag->y_coord, &hit_result, &p);
        Color r = p.bg;
        r = vec3_mul(r,params->reflection_amount*fresnel);
        surface = vec3_add(surface, r);
        surface = vec3_saturate(surface);

        //surface = vec3_lerp(surface, r, params->reflection_amount);
    }

    if (params->fog_enabled) {
        float fog = 1.0 - calculate_fog(renderSettings,frag);
        surface = vec3_lerp(surface, renderSettings->background_color, fog);
    }

    // DEBUG OUT
    if (renderSettings->render_mode == RENDERMODE_DEBUG){
        _ascii_shader_blocks(surface,surface,1.0f,out_pixel);
        return;
    }
    
    
    surface = dither(surface,frag->x_coord, frag->y_coord, params->dither_amount);
    surface = vec3_saturate(surface);

    // if there is a lut we apply it otherwise we output raw colors
    if (params->lut == NULL){
        _ascii_shader_full(surface,out_pixel);
    }
    else{
        // magic number from trial and error
        //surface = vec3_pow(surface, 0.75f);
        AsciiPixel best = _lookup_lut(params->lut, surface);
        out_pixel->fg = best.fg;
        out_pixel->bg = best.bg;
        memcpy(out_pixel->bytes, best.bytes, 3*sizeof(char));
    }
    return;
}











/*


// glitchy 01

void shader_screen_space_glitch(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParamsScreenSpaceGlitch *params = (ShaderParamsScreenSpaceGlitch*)payload;

    TextureImage *img = renderSettings->textures[params->texture];
    Vec2 uv = screen_coordinates_to_uv(renderSettings, frag->x_coord, frag->y_coord);
    Vec3 tex = sample_texture(renderSettings, params->texture,uv.x,uv.y);
    
    tex = vec3_quantize(tex,4);

    float x = (float)frag->x_coord;
    float y = (float)frag->y_coord;
    
    float threshold_map = make_glitchy_dithering(tex,x,y,0.25f).x;
    tex.x += threshold_map * params->dither_amount;
    tex.y += threshold_map * params->dither_amount;
    tex.z += threshold_map * params->dither_amount;

    AsciiPixel best = _lookup_lut(params->lut, tex);
    out_pixel->fg = best.fg;
    out_pixel->bg = best.bg;
    memcpy(out_pixel->bytes, best.bytes, 3*sizeof(char));
}

*/




// for n64 matcap shaders
void shader_matcap(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    Color surface = sample_matcap(renderSettings, 0, frag->normal);
    Color hsv = rgb2hsv(surface);
    float luma = dither_float(hsv.z,frag->x_coord,frag->y_coord, 0.2, 0);
    hsv.z = 1.f;
    Color c= hsv2rgb(hsv);
    _ascii_shader_blocks(c, renderSettings->background_color, luma, out_pixel);

}




void shader_surface_chrome_environment( RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Surface_Chrome *params = payload;

    Vec3 reflect = vec3_reflect(frag->view_direction, frag->normal); 
    Color surface = sample_environmnet(renderSettings, reflect, params->tex_id);
    surface = vec3_mul_vec3(surface,params->tint);
    
    //float spec = bsdf_simple_mono(renderSettings, frag, 0.0f, 0.0f,1.5f,0.95f, false, 0);
    //surface.x += spec;
    //surface.y += spec;
    //surface.z += spec;
        
    if (renderSettings->render_mode == RENDERMODE_DEBUG){
        _ascii_shader_full(surface, out_pixel);
        return;
    }

    surface = dither(surface,frag->x_coord,frag->y_coord,params->dither_amount);
    if (params->lut != NULL)
    {
        _ascii_shader_from_lut(surface, params->lut, out_pixel);
        return;
    }
    _ascii_shader_full(surface, out_pixel);

}




void shader_surface_chrome( RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Surface_Chrome *params = payload;

    HitResult hit_result = {0};
    cast_reflection_ray(renderSettings, frag->pos, frag->normal, frag->view_direction, &hit_result);
    process_ray(renderSettings,frag->x_coord, frag->y_coord, &hit_result, out_pixel);
    out_pixel->fg = vec3_mul_vec3(out_pixel->fg, params->tint);
    out_pixel->bg = vec3_mul_vec3(out_pixel->bg, params->tint);
    //_ascii_shader_full(surface, out_pixel);

}






static inline Vec2 screen_coordinates_to_uv_pixel_exact(size_t x, size_t y, TextureImage *img)
{
    float u = x % img->width;
    float v = y % img->height;
    u /= img->width;
    v /= img->height;
    return (Vec2){u,1.0f-v};
}
static inline Vec2 screen_coordinates_to_uv(RenderSettings *renderSettings, size_t x, size_t y)
{
    float u = x;
    float v = y;
    u /= renderSettings->render_width;
    v /= renderSettings->render_height;
    return (Vec2){u,1.0f-v};
}
void shader_environment_texture(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Environment_Texture *params = payload;

    Color color = {0};    

    switch(params->method)
    {
        case ENV_SPHERICAL:
            color = sample_environmnet(renderSettings, frag->view_direction, params->tex_id);
            break;
        case ENV_SCREEN_PIXEL_PERFECT:
            TextureImage *img = renderSettings->textures[params->tex_id];
            Vec2 uv = screen_coordinates_to_uv_pixel_exact(frag->x_coord, frag->y_coord, img);
            color = sample_texture(renderSettings, params->tex_id,uv.x,uv.y);
            break;
    }

    color = vec3_mul(color,params->brightness);
    if (renderSettings->render_mode == RENDERMODE_DEBUG){
        _ascii_shader_full(color, out_pixel);
        return;
    }
    
    color = dither(color,frag->x_coord,frag->y_coord,params->dither_amount);
    color = vec3_saturate(color);
    if (params->lut != NULL) 
    {
        _ascii_shader_from_lut(color, params->lut, out_pixel);
        return;
    }

    _ascii_shader_full(color, out_pixel);
}











void shader_sprite_principled(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    
    ShaderParams_Sprite_Principled *params = payload;

    float alpha = sample_texture(renderSettings, params->opacity_tex_id, frag->uv.x, frag->uv.y).x;
    if (alpha < 0.5f) return;

    out_pixel->bg = blend(out_pixel->bg, params->color, params->bg_blend_method);
    out_pixel->fg = blend(out_pixel->fg, params->color, params->fg_blend_method);

    switch (params->ascii_blend_method)
    {
        case BLEND_METHOD_ASCII_FILL:
            memcpy( out_pixel->bytes, BLK_SOLID, 3*sizeof(char));
            break;
    }
}






// this shader assumes that the color buffer holds the full output color in the FG (and BG)
void shader_post_lut(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_LUT *params = payload;
    
    Color color = out_pixel->fg;
    if (luma(color) > 0.05f) color = dither(color, frag->x_coord, frag->y_coord, params->dither_amount);
    //color = vec3_pow(color, 0.75f);
    AsciiPixel best = _lookup_lut(params->lut, color);
    out_pixel->fg = best.fg;
    out_pixel->bg = best.bg;
    memcpy(out_pixel->bytes, best.bytes, 3*sizeof(char));
}






float cheap_gaussian_weight(int i, int j) {
    float dist_sq = (float)(i * i + j * j);
    float weight = 1.0f - (0.01f * dist_sq);
    return weight < 0.0f ? 0.0f : weight;
}




// this shader assumes that the color buffer holds the full output color in the FG (and BG)
// it checks if stencil == 1 for light mateial
void shader_post_bloom(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_Bloom *params = payload;
    Color color = out_pixel->fg;
    Color bloom = {0};
    
    uint8_t num_samples = 16;
    float weight_accum = 0;

    
    for (int16_t i=-(num_samples); i<num_samples+1; i++){
        for (int16_t j=-num_samples; j<num_samples+1; j++){

            float weight = cheap_gaussian_weight(i,j);
            if (weight <= 0.00001) continue;
            int16_t x = (int16_t)frag->x_coord + i*4;
            int16_t y = (int16_t)frag->y_coord + j*2;
            uint8_t stencil = sample_stencil_buffer(renderSettings,x,y);

            if (stencil==1)
            {
                Color sample = sample_canvas(renderSettings, x,y)->fg;
                sample = vec3_mul(sample, weight);
                bloom = vec3_add(bloom, sample);
            }
            weight_accum += weight;
        }
    }
    bloom = vec3_div(bloom,weight_accum);
    bloom = vec3_mul(bloom,10.0);
    color = vec3_add(color, bloom);
    if (luma(color) > 0.01)
    { 
        color = dither(color, frag->x_coord, frag->y_coord, params->dither_amount);
    }
    color = vec3_saturate(color);
   
    if (params->lut != NULL) _ascii_shader_from_lut(color, params->lut, out_pixel); 
    else                     _ascii_shader_full(color, out_pixel);
}





void shader_post_blur(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_Blur *params = payload;
    
    Color color = out_pixel->fg;
    
    //Color bg_accum = {0};
    //Color fg_accum = {0};
    Color color_accum = {0};
    float weight_accum = 0;
    
    float r = hash11(frag->y_coord);
    if (r>0.5) return;

    //uint8_t stencil = sample_stencil_buffer(renderSettings,frag->x_coord,frag->y_coord);
    //if (stencil != 1) return;
    
    for (int16_t i=-params->x_samples*2; i<params->x_samples*2+1; i++){
        for (int16_t j=-params->y_samples; j<params->y_samples+1; j++){

            float weight = cheap_gaussian_weight(i,j);
            if (weight <= 0.00001) continue;
            int16_t x = (int16_t)frag->x_coord + i*1;
            int16_t y = (int16_t)frag->y_coord + j*1;
            //uint8_t stencil = sample_stencil_buffer(renderSettings,x,y);
            //if (stencil==1)
            {
                AsciiPixel *sample = sample_canvas(renderSettings, x,y);
                Color bg = vec3_mul(sample->bg, weight);
                Color fg = vec3_mul(sample->fg, weight);
                Color mix = vec3_lerp(bg,fg,0.5f);
                color_accum = vec3_add(color_accum, bg);
            }
            weight_accum += weight;
        }
    }
    color = vec3_div(color_accum,weight_accum);
    
    //if (luma(color) > 0.05)
    { 
        color = dither(color, frag->x_coord, frag->y_coord, params->dither_amount);
    }
    color = vec3_saturate(color);
    //_ascii_shader_full(color, out_pixel);
    //return;
    if (params->lut != NULL)
    {
        AsciiPixel temp = {0};
        _ascii_shader_from_lut(color, params->lut, &temp); 
        memcpy(out_pixel->bytes, temp.bytes, 3*sizeof(char));
        out_pixel->bg = temp.bg;
        //out_pixel->fg = temp.fg;
    }
    else _ascii_shader_full(color, out_pixel);
}








void shader_post_dispersion(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_Dispersion *params = payload;
    Color color = out_pixel->fg;
    
    //Color bg_accum = {0};
    //Color fg_accum = {0};
    Color color_accum = {0};
    float weight_accum = 0;
    
    float r = hash11(frag->y_coord);
    if (r<params->dropout_chance){
        return;
    }
    float x = remap_from01(r, 0.5f, 1.25f);
    
    static float iorR = 1.15f;
    static float iorG = 1.18f;
    static float iorB = 1.22f;

    //uint8_t stencil = sample_stencil_buffer(renderSettings,frag->x_coord,frag->y_coord);
    //if (stencil != 1) return;
    Vec3 direction = {1.0f, 0.0f};
    direction = vec3_mul(direction, params->size * x);
    //Vec3 direction = {renderSettings->canvas.width/2 - frag->x_coord, renderSettings->canvas.height/2 - frag->y_coord, 0.0f}; 
    //direction = vec3_mul(direction, 0.0125f);

    direction = vec3_div(direction, params->num_samples);
    
    for (int16_t i=0; i<params->num_samples; i++)
    {
        float slide = i;
        
        //float weight = cheap_gaussian_weight(i,i);
        //if (weight <= 0.00001) continue;
        
        Color bg = {0};
        Color fg = {0};

        Vec2 r_uv = {frag->x_coord + direction.x * slide * 1.0f, frag->y_coord + direction.y * slide * 1.0f};
        Vec2 g_uv = {frag->x_coord + direction.x * slide * 2.0f, frag->y_coord + direction.y * slide * 2.0f};
        Vec2 b_uv = {frag->x_coord + direction.x * slide * 3.0f, frag->y_coord + direction.y * slide * 3.0f};
        
        r_uv.x -= direction.x*params->num_samples/2;
        g_uv.x -= direction.x*params->num_samples/2;
        b_uv.x -= direction.x*params->num_samples/2;

        AsciiPixel *sample_r = sample_canvas(renderSettings, r_uv.x, r_uv.y);
        AsciiPixel *sample_g = sample_canvas(renderSettings, g_uv.x, g_uv.y);
        AsciiPixel *sample_b = sample_canvas(renderSettings, b_uv.x, b_uv.y);
        
        bg.x = sample_r->bg.x;
        bg.y = sample_g->bg.y;
        bg.z = sample_b->bg.z;

        fg.x = sample_r->fg.x;
        fg.y = sample_g->fg.y;
        fg.z = sample_b->fg.z;
        
        //c = vec3_mul(c, weight);
        Color mix = vec3_lerp(fg,bg,0.5f);
        color_accum = vec3_add(color_accum, mix);
        weight_accum += 1.0f; //weight;
    }
    color_accum = vec3_div(color_accum,weight_accum);
    color = color_accum;
    
    if (luma(color) > 0.1f)
    { 
        color = dither(color, frag->x_coord, frag->y_coord, params->dither_amount);
    }
    color = vec3_saturate(color);
    if (params->lut == NULL){
        _ascii_shader_full(color, out_pixel);
        return;
    }
    AsciiPixel temp = {0};
    _ascii_shader_from_lut(color, params->lut, &temp); 
    //memcpy(out_pixel->bytes, temp.bytes, 3*sizeof(char));
    _replace_only_empty_char(out_pixel, temp.bytes);
    out_pixel->bg = temp.bg;
    out_pixel->fg = temp.fg;
}


void shader_post_horizontal_glitches(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_Horizontal_Glitches *params = payload;
    
    Color color = {0};
    float weight_accum = 0;
    
    // Check current pixel's stencil
    uint8_t current_stencil = sample_stencil_buffer(renderSettings, frag->x_coord, frag->y_coord);
    if (current_stencil == 1) return;

    bool found_stencil = false;
    
    for (int i = 0; i < params->num_samples; i++)
    {
        // Random vertical dropouts
        float r = hash11((float)frag->y_coord + 33.34f * i);
        if (r < params->dropout_chance) continue;

        int16_t x = (int16_t)frag->x_coord - i * params->stride;
        
        // FIX: Instead of MAX(x, 1), skip samples that fall off the left side of the screen
        if (x < 0) break; // Or 'continue', but since 'i' only increases, 'break' optimizes it

        int16_t y = (int16_t)frag->y_coord;
        
        // FIX: Renamed to avoid shadowing the upper 'stencil' variable
        uint8_t sampled_stencil = sample_stencil_buffer(renderSettings, x, y); 
        
        if (sampled_stencil == 1){
            found_stencil = true;
            AsciiPixel *sample = sample_canvas(renderSettings, x, y);
            Color tint = color_random_from_palette(C_ANSI_PALETTE, 16, i);
            
            // Optionally darken the copy
            Color c = vec3_lerp(sample->bg, sample->fg, 0.5f);
            float l = luma(c);
            l = (1.0f + l) / 2.0f;
            tint = vec3_mul(tint, l);
            
            color = vec3_add(color, tint);
            weight_accum += 1.0f;
        }
    }
    
    if (!found_stencil || weight_accum == 0.0f) return;

    color = vec3_div(color, weight_accum);

    #if 0
    color = vec3_saturate(color);
    _ascii_shader_full(color, out_pixel);
    return;
    #endif

    color = dither(color, frag->x_coord, frag->y_coord, params->dither_amount);
    color = vec3_saturate(color);
    if (params->lut != NULL)
    {
        AsciiPixel temp = {0};
        _ascii_shader_from_lut(color, params->lut, &temp); 
        memcpy(out_pixel->bytes, temp.bytes, 3*sizeof(char));
        out_pixel->bg = temp.bg;
        out_pixel->fg = temp.fg;
    }
    else _ascii_shader_full(color, out_pixel);
}




void shader_post_horizontal_blur(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_Horizontal_Glitches *params = payload;
    
    Color color = {0};
    float weight_accum = 0;
    uint8_t stencil = sample_stencil_buffer(renderSettings,frag->x_coord,frag->y_coord);
    if (stencil==1) return;
    
    //float r = hash11(frag->y_coord);
    //if (r>0.5) return;
    bool found_stencil = false;
    
    for (int i=0; i<params->num_samples; i++)
    {

        float weight = 1.0f / ((float)i/params->num_samples);//cheap_gaussian_weight(i,0);
        if (weight<= 0.001) break;

        int16_t x = (int16_t)frag->x_coord - i*params->stride;
        int16_t y = (int16_t)frag->y_coord + 0;
        AsciiPixel *sample = sample_canvas(renderSettings, x,y);
        Color c = vec3_lerp(sample->bg, sample->fg, 0.5f);
        color = vec3_add(color, c);
        weight_accum += 1;
        //weight_accum += weight;
        uint8_t stencil = sample_stencil_buffer(renderSettings,x,y);
        if (stencil==1){
            found_stencil=true;
            break;
        }
    }
    if (!found_stencil) return;

    color = vec3_div(color, weight_accum);

    #if 1
    //color = vec3_saturate(color);
    //_ascii_shader_full(color, out_pixel);
    //return;
    #endif

    color = dither(color, frag->x_coord, frag->y_coord, params->dither_amount);
    color = vec3_saturate(color);
    if (params->lut != NULL)
    {
        AsciiPixel temp = {0};
        _ascii_shader_from_lut(color, params->lut, &temp); 
        memcpy(out_pixel->bytes, temp.bytes, 3*sizeof(char));
        out_pixel->bg = temp.bg;
        //out_pixel->fg = temp.fg;
    }
    else _ascii_shader_full(color, out_pixel);
}











void shader_post_horizontal_shifts(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_Horizontal_Glitches *params = payload;
    float r = hash11(frag->y_coord);
    if (r>0.5) return;
        int16_t x = (int16_t)frag->x_coord + params->stride;
        int16_t y = (int16_t)frag->y_coord + 0;
        AsciiPixel *sample = sample_canvas(renderSettings, x,y);
        memcpy(out_pixel->bytes, sample->bytes, 3*sizeof(char));
        out_pixel->bg = sample->bg;
        out_pixel->fg = sample->fg;
    return;
}



// simply repeats copies the pixels and repeats them
void shader_post_horizontal_repeat(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_Horizontal_Glitches *params = payload;
    Color color = out_pixel->fg;
    
    //Color bg_accum = {0};
    //Color fg_accum = {0};
    Color color_accum = {0};
    float weight_accum = 0;
    uint8_t stencil = sample_stencil_buffer(renderSettings,frag->x_coord, frag->y_coord);
    if (stencil==1) return;
    
    float r = hash11(frag->y_coord);
    //if (r>0.5){
    //    return;
   // }
    Vec3 direction = {1.0f, 0.0f};
    direction = vec3_mul(direction, params->stride);
    
    for (int16_t i=0; i<params->num_samples; i++)
    {
        float x = frag->x_coord - i*direction.x;
        float y = frag->y_coord - i*direction.y;
        
        uint8_t stencil = sample_stencil_buffer(renderSettings,x,y);
        if (stencil!=1) continue;
        Color bg = {0};
        Color fg = {0};
        AsciiPixel *sample = sample_canvas(renderSettings, x,y);
        //Color s = vec3_lerp(sample->bg, sample->bg, 0.5f);
        Color c = color_random_from_palette(C_ANSI_PALETTE,16, x*y);
        out_pixel->bg = sample->bg;
        out_pixel->fg = sample->fg;
        
        memcpy(out_pixel->bytes, sample->bytes, 3*sizeof(char));
        //color_accum = vec3_add(color_accum, s);
        //color_accum = vec3_add(color_accum, c);
        //weight_accum += 1.0f; //weight;
        break;
    }
    return;
    color_accum = vec3_div(color_accum,weight_accum);
    color = color_accum;
    
    if (luma(color) > 0.1f)
    { 
        color = dither(color, frag->x_coord, frag->y_coord, params->dither_amount);
    }
    color = vec3_saturate(color);

    if (params->lut == NULL){
        //_ascii_shader_full(color, out_pixel);
        //return;
    }
    AsciiPixel temp = {0};
    //_ascii_shader_from_lut(color, params->lut, &temp); 
    _ascii_shader_full(color, &temp);
    //memcpy(out_pixel->bytes, temp.bytes, 3*sizeof(char));
    //_replace_only_empty_char(out_pixel, temp.bytes);
    out_pixel->bg = temp.bg;
    //out_pixel->fg = temp.fg;
}
