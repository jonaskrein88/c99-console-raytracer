
#include <math.h>
#include <float.h>
#include <stdint.h>
#include "vector.h"
#include "shader_utils.h"
#include "raymarching.h"



static inline float smoothMin(float d1, float d2, float k) {
    float h = clamp01(0.5 + 0.5 * (d2 - d1) / k);
    return lerpf(d2, d1, h) - k * h * (1.0 - h);
}

// Distance function for a single sphere
static inline float sdSphere(Vec3 p, float radius) {
    return vec3_magnitude(p) - radius;
}

// The global Scene Description (The Map function)
static float map(Vec3 p, float time, size_t num_colors, Color *colors, Color *out_color)
{
    float gooeyness = 0.85f;
    float scene = FLT_MAX;
    float weight_accum = 0.0f;
    Color color_accum = {0};
    
    for (size_t i=0; i<24; i++)
    {
        float t = time + i;
        float freq_x = remap_from01(hash11(i*23.232), 1.0,1.75f);
        float freq_y = remap_from01(hash11(i*76.83),  1.0,1.75f);
        float freq_z = remap_from01(hash11(i*343.2),  1.0,1.75f);
        
        float range_x = remap_from01(hash11(i*34.34), 1.25f, 3.75f);
        float range_y = remap_from01(hash11(i*77.67), 0.75f, 1.75f);
        float range_z = remap_from01(hash11(i*245.62), 1.0f, 1.5f);

        float radius = remap_from01(hash11(i*756.4), 0.35f, 0.75f);

        Vec3 ball = {sin(t * freq_x) * range_x, cos(t * freq_y) * range_y, cos(t * freq_z) * range_z};
    
        float d = sdSphere(vec3_sub(p, ball), radius);
        scene = smoothMin(scene, d, gooeyness);

        Color color = {0};
        if (num_colors > 0) {
            float c = hash11(i*677.656f);
            //color = gradient_map_interpolated(c, colors, num_colors);
            uint8_t i = fmodf(floor(c*num_colors),num_colors);
            color = colors[i];
            float w = expf(-d / 0.5f); 
            color_accum = vec3_add(color_accum, vec3_mul(color,w));
            weight_accum += w;
        }
    }
    
    if (num_colors > 0) {
        *out_color = vec3_div(color_accum,weight_accum);
    }

    
    return scene;
}


static Vec3 getNormal(Vec3 p, float time) {
    Vec3 e = {0.001, 0.0f, 0.0f};
    float d = map(p,time,0,NULL,NULL);
    Vec3 a = vec3_xyy(e); 
    Vec3 b = vec3_yxy(e); 
    Vec3 c = vec3_yyx(e); 

    Vec3 n = vec3_sub((Vec3){d,d,d},(Vec3){
        map(vec3_sub(p,a),time,0,NULL,NULL),
        map(vec3_sub(p,b),time,0,NULL,NULL),
        map(vec3_sub(p,c),time,0,NULL,NULL)
    });
    return vec3_normalize(n);
}



void shader_post_metaballs(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_MetaBalls *params = payload;
    
    float ambient_amount = params->ambient_amount;
    float specular_amount = params->specular_amount;
    float diffuse_amount = params->diffuse_amount;

    float time = renderSettings->current_frame/ 150.f;
    float u = frag->uv.x-0.5;
    float v = frag->uv.y-0.5;
    u *= 2.0;
   
    // i tried the actual camera position but it didn't work
    Vec3 ro = {0.0, 0.0, -5.0};                  // ray origin
    Vec3 rd = vec3_normalize((Vec3){u, v, 1.0}); // ray direction
    
   float t = 0.0; 
    int hit = 0;
    int i;
    Color c;
    
    for (i = 0; i < 80; i++) {
        Vec3 p = vec3_add(ro, vec3_mul(rd, t)); 
        float d = map(p, time, params->num_colors, params->colors, &c);     
        
        if (d < 0.01f) { 
            hit = 1; // flag the hit
            break; 
        }
        if (t > 10.0f) break;  
        
        t += d; 
    }
   
    

    // Shading
    Vec3 color = {0.0f, 0.0f, 0.0f};
    
    if (hit) {
        Vec3 p = vec3_add(ro, vec3_mul(rd, t)); 
        Vec3 n = getNormal(p, time);

        Vec3 baseColor = c;
        float gradient = remap_to01(t,3.5f, 6.5f);
        gradient = pow(gradient,0.85f);
        
   
        Vec3 viewDir = vec3_normalize(vec3_sub(ro, p)); 
        Vec3 lightPos0 = {5.0, 7.0, -2.0};
        Vec3 lightPos1 = {-5.0, 3.0, -2.0};
        Vec3 lights[] = {lightPos0, lightPos1};

        
        color = vec3_mul(baseColor, ambient_amount);
        
        for (int i = 0; i<2; i++)
        {
        
            // surface to light
            Vec3 lightPos = lights[i];
            Vec3 lightDir = vec3_normalize(vec3_sub(lightPos, p));
            float diff = vec3_dot(n, lightDir);
            //diff *= 0.5;
            //diff += 0.5f;
            //diff = pow(diff,2);

            diff = clamp01(diff*diffuse_amount);
            
            
            Vec3 invLightDir = vec3_negate(lightDir);
            Vec3 reflectDir = vec3_reflect(invLightDir, n);
            
            // specular
            float spec = pow(fmaxf(vec3_dot(viewDir, reflectDir), 0.0f), 64.0f);
            spec = clamp01(spec * specular_amount);
            
            Vec3 specularColor = {1.0, 1.0, 1.0}; 
            Vec3 diffuseResult = vec3_mul(baseColor, diff);
            Vec3 specularResult = vec3_mul(specularColor, spec);
            Vec3 total = vec3_add(diffuseResult, specularResult);

            color = vec3_add(total, color);
        }
        //color = vec3_add(color, baseColor);
        color = vec3_div(color,2.0f);

        // add fresnel for rim light effect
        //float f = calculate_fresnel(n, viewDir, 0.2f);
        //color = vec3_add(color, vec3_mul(baseColor, f*0.5f));

        // Gamma correction
        color = vec3_pow(color, 1.0 / 2.2);

        // multiply by depth for fog effect
        color = vec3_mul(color, pow(1.0f-gradient,0.45));

        if (renderSettings->render_mode == RENDERMODE_DEBUG) {
            color = vec3_saturate(color);
            _ascii_shader_full(color,out_pixel);
            return;
        }

        color = dither(color, frag->x_coord, frag->y_coord, params->dither_amount);
        color = vec3_saturate(color);
        _ascii_shader_from_lut(color, params->lut, out_pixel);
    }
}




Material ce_new_material_post_metaballs(AsciiLUT *lut, size_t num_balls, Color colors[], size_t num_colors, float ambient, float diffuse, float specular,float dither_amount){
    ShaderParams_Post_MetaBalls *params = calloc(1, sizeof(ShaderParams_Post_MetaBalls));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount=dither_amount;
    params->colors = colors;
    params->num_balls = num_balls;
    params->num_colors = num_colors;
    params->specular_amount = specular;
    params->diffuse_amount = diffuse;
    params->ambient_amount = ambient;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_post_metaballs
    };
    return new;
}
















// Black Hole
// based on this shadertoy:
// shadertoy.com/view/tX2fR1





const float MAX_DIST = 16.0; 
const float STEP_RANDOMIZATION = 1.0;
const int MAX_ITER = 64;
const float MASS = 0.15;
const float DENSITY = 10.0;

Vec2 getRadialUv(Vec3 uv) 
{
    float angle = atan2f(uv.x, -uv.y);
    Vec2 radialUv = {0};
    radialUv.x = angle / (3.14159265f * 2.0f) + 0.5f;
    radialUv.y = 1.0f - powf(1.0f - vec3_magnitude(uv), 4.0f);
    return radialUv;
}

typedef struct BlackHoleRay {
    Vec3 ro;
    Vec3 rd;
    Vec3 hitPos;
    float dist;
    float density;
    float stepSize;
    Vec3 prevPos;
    float dopplerFactor; 
} BlackHoleRay;

float sdTorus(Vec3 p, float r1, float r2) 
{
    float xz_len = sqrtf(p.x * p.x + p.z * p.z);
    float q_x = xz_len - r1;
    float q_y = p.y;
    return sqrtf(q_x * q_x + q_y * q_y) - r2;
}

float black_hole_scene(Vec3 p) 
{
    Vec3 q = p;
    q.y *= 16.0f; 
    
    return sdTorus(q, 0.65f, 0.3f);
}

BlackHoleRay attractRay(BlackHoleRay ray, float stepDist) 
{
    float l = vec3_magnitude(ray.hitPos);
    
    if (l > 0.1f) {
        float gravity_scale = (stepDist * MASS) / (l * l * l);
        Vec3 pull = vec3_mul(ray.hitPos, -gravity_scale);
        
        ray.rd = vec3_normalize(vec3_add(ray.rd, pull));
    }
    return ray;
}


BlackHoleRay Raymarch(Vec3 ro, Vec3 rd, Vec2 uv, float sample_seed) 
{
    BlackHoleRay ray;
    ray.ro = ro;
    ray.rd = rd;
    ray.hitPos = ro;
    ray.dist = 0.0f;
    ray.density = 0.0f;
    ray.dopplerFactor = 0.5f; 

    float stepDist = MAX_DIST / (float)MAX_ITER;
    float initial_jump = 4.0f; 
    
    Vec2 jitter_uv = {uv.x + sample_seed, uv.y - sample_seed};
    initial_jump += hash22(jitter_uv).x * stepDist * STEP_RANDOMIZATION;
    
    ray.hitPos = vec3_add(ray.hitPos, vec3_mul(ray.rd, initial_jump));
    ray.dist += initial_jump;

    float max_contribution = -1.0f;
    float best_intersection_x = 0.0f;

    for (int i = 0; i < MAX_ITER; i++) 
    {
        // bend ray
        ray = attractRay(ray, stepDist);
        
        // advance ray
        ray.hitPos = vec3_add(ray.hitPos, vec3_mul(ray.rd, stepDist));

        float dist = black_hole_scene(ray.hitPos);

        // bigger threshold for sharper edges
        if (dist < 0.5f) {
            float contribution = fmaxf(0.0f, 1.0f - (dist / 0.5f));
            ray.density += stepDist * contribution; 
            
            if (contribution > max_contribution) {
                max_contribution = contribution;
                best_intersection_x = ray.hitPos.x;
            }
        }

        ray.dist += stepDist;
        
        if (vec3_magnitude(ray.hitPos) < 0.2f) {
            break;
        }
    }

    ray.dopplerFactor = clamp01(best_intersection_x / 1.0f);
    return ray;
}

Vec2 environment_uv_from_rd(Vec3 rd)
{
    Vec2 env_uv;
    env_uv.x = atan2f(rd.z, rd.x) / (2.0f * 3.14159265f) + 0.5f;
    env_uv.y = asinf(rd.y) / 3.14159265f + 0.5f;
    return env_uv;
}
void shader_post_black_hole(RenderSettings *renderSettings, FragmentData *frag, void *payload, AsciiPixel *out_pixel)
{
    ShaderParams_Post_BlackHole *params = payload;
    
    Vec3 uv = frag->uv;
    uv.x -= 0.5f;
    uv.x /= 2.0f;
    uv.y -= 0.5f;
    uv.y *= (float)renderSettings->canvas.height / (float)renderSettings->canvas.width;
    uv.x *= 2.0f;
    uv.y *= 2.0f;
    
    Matrix r = matrix_fromXRotation(renderSettings->camera.pos.y/100.0f);
    
    Vec3 ro = {0.0f, 0.0f, -12.0f};
    Vec3 rd = vec3_normalize((Vec3){uv.x, uv.y, 4.0f});

    rd = vec3_transform(&r,rd);
    ro = vec3_transform(&r,ro);
    
    Color out = {0};
    //static Color dark_orange = {0.2, 0.15, 0.075};
    //static Color cyan = {0.5,1,1};
    static int num_samples = 128; 
    for (int i=0; i<num_samples; i++)
    {
        BlackHoleRay ray = Raymarch(ro, rd, (Vec2){uv.x, uv.y}, (float)i*67.78);
        if (ray.density > 0.05)
        {
            float x = clamp01 (ray.hitPos.x/1.0f);
            Color c = vec3_lerp(params->dark_color, params->bright_color, ray.dopplerFactor);
            Color col = vec3_mul(c, ray.density * DENSITY);
            col = vec3_pow(col,0.45);
            out = vec3_add(out,col);
        }
        // scratched the environment
        // looks bad in low resolution
        else {
            //Vec2 uv = environment_uv_from_rd(ray.rd);
            //Color tex = sample_texture_bilinear(renderSettings, params->tex_id, uv.x, uv.y);
            //out = vec3_add(out,tex);
        }

    }
    out = vec3_div(out,num_samples);

    if (renderSettings->render_mode == RENDERMODE_DEBUG){
        out = vec3_saturate(out);
        _ascii_shader_full(out, out_pixel);
        return;
    }

    // quick check so we don't get ugly dithering in the black areas
    if (luma(out)>0.01f)
    {
        out = dither(out, frag->x_coord, frag->y_coord, params->dither_amount);
        out = vec3_saturate(out);
        _ascii_shader_from_lut(out, params->lut, out_pixel);
    }

}

Material ce_new_material_post_black_hole(AsciiLUT *lut, Color dark_color, Color bright_color, float dither_amount, size_t tex_id)
{
    ShaderParams_Post_BlackHole *params = calloc(1, sizeof(ShaderParams_Post_BlackHole));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount=dither_amount;
    params->dark_color = dark_color;
    params->bright_color = bright_color;
    params->tex_id = tex_id;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_post_black_hole
    };
    return new;
}







