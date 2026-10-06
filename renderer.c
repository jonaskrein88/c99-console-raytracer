
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <conio.h>
#include <windows.h>
#include <float.h>

#include "renderer.h"
#include "vector.h"
#include "importer.h"
#include "objects.h"
#include "shaders.h"
#include "bvh.h"



static const float PI = 3.14159f;








void camera_update_transform(Camera *cam) {
    float max_pitch = 89.0f * (PI / 180.0f);
    if (cam->pitch > max_pitch)  cam->pitch = max_pitch;
    if (cam->pitch < -max_pitch) cam->pitch = -max_pitch;

    Vec3 _forward;
    _forward.x = cosf(cam->pitch) * sinf(cam->yaw);
    _forward.y = sinf(cam->pitch);
    _forward.z = cosf(cam->pitch) * cosf(cam->yaw);
    cam->forward = vec3_normalize(_forward);

    // TARGET CAM
    // - align to target
    if (cam->is_targeted) {
        cam->forward = vec3_normalize( vec3_sub( cam->target, cam->pos) );
    }
    
    Vec3 _up = {0.0f, 1.0f, 0.0f};
    cam->right = vec3_normalize(vec3_cross(cam->forward, _up));
    cam->up    = vec3_normalize(vec3_cross(cam->right, cam->forward));

    // pitch and yaw are not really used when targeted but we still calculate them for information
    if (cam->is_targeted) {
        Matrix m = camera_get_transform(cam);
        Vec3 r = matrix_get_euler_angles(&m);
        cam->pitch = r.y;
        cam->yaw = r.x;
    }
}

void camera_init(Camera *cam, uint8_t width, uint8_t height)
{
    cam->pos    = (Vec3){0.f, 1.f, -25.f};
	cam->up     = (Vec3){0.f, 1.f, 0.f};
    cam->right  = (Vec3){1.f, 0.f, 0.f};
    cam->forward = (Vec3){0.f, 0.f, 1.f};
    cam->yaw   = 0.f;
    cam->pitch = 0.f;
	cam->target = (Vec3){0.f, 0.5f, 0.f};
    cam->focalDist = 1.0f;
    cam->sensorWidth = 2.5f;
    cam->ortho_size = 10.0f;
    cam->sensorRatio = (float)width / height;
    cam->sensorHeight = cam->sensorWidth / cam->sensorRatio;
    cam->is_targeted = true;
    cam->is_orthographic = false;
    
    camera_update_transform(cam);
}

void camera_walk(Camera *cam, float value) {
    Vec3 offset = vec3_mul(cam->forward, value);
    cam->pos    = vec3_add(cam->pos, offset);
    camera_update_transform(cam);
}
void camera_strafe(Camera *cam, float value) {
    Vec3 offset = vec3_mul(cam->right, value);
    cam->pos    = vec3_add(cam->pos, offset);
    camera_update_transform(cam);
}
void camera_strafe_vertical(Camera *cam, float value) {
    Vec3 offset = vec3_mul(cam->up, value);
    cam->pos    = vec3_add(cam->pos, offset);
    camera_update_transform(cam);
}
void camera_rotate_yaw(Camera *cam, float value) {
    cam->yaw += value;
    camera_update_transform(cam);
}
void camera_rotate_pitch(Camera *cam, float value){
    cam->pitch += value;
    camera_update_transform(cam);
}

void camera_zoom(Camera *cam, float value) {
    if (cam->is_orthographic) cam->ortho_size += value * .1f;
    else                      cam->focalDist += value * .1f;
}

bool camera_toggle_orthographic_projection(Camera *cam)
{
    cam->is_orthographic = !cam->is_orthographic;
    return cam->is_orthographic;
}

Matrix camera_get_transform(Camera *cam)
{
    Matrix out = {0};
    out.m[0] = cam->right.x;
    out.m[1] = cam->up.x;
    out.m[2] = cam->forward.x;
    out.m[3] = cam->pos.x;

    out.m[4] = cam->right.y;
    out.m[5] = cam->up.y;
    out.m[6] = cam->forward.y;
    out.m[7] = cam->pos.y;

    out.m[8] = cam->right.z;
    out.m[9] = cam->up.z;
    out.m[10] = cam->forward.z;
    out.m[11] = cam->pos.z;
    return out;
}

Ray camera_ray(Camera *cam, float u, float v)
{
    if (cam->is_orthographic)
    {
        Vec3 a = vec3_mul(cam->right, u * cam->ortho_size);
        Vec3 b = vec3_mul(cam->up,    v * cam->ortho_size/2.0f);
        Vec3 origin = vec3_add(cam->pos, vec3_add(a,b));
        return (Ray){.direction=vec3_normalize(cam->forward), .origin=origin};
    }
    else 
    {
        Vec3 a = vec3_mul(cam->right, u * cam->sensorWidth);
        Vec3 b = vec3_mul(cam->up, v * cam->sensorHeight);
        Vec3 c = vec3_mul(cam->forward, cam->focalDist);
        Vec3 dir = vec3_normalize( vec3_add( vec3_add(a,b),c) );
        return (Ray){.direction=dir, .origin=cam->pos};
    }
}








Light ce_new_point_light(float power, Color color, Vec3 pos)
{
    Light light = {
        .pos=pos,
        .color=color,
        .power=power,
    };
    return light;
}

static Vec3 _light_direction_from_height_and_angle(float height, float angle) {
    float y = fmaxf(0.0f, fminf(1.0f, height));
    float horizontal_radius = sqrtf(fmaxf(0.0f, 1.0f - (y * y)));
    float x = horizontal_radius * cosf(angle);
    float z = horizontal_radius * sinf(angle);
    return (Vec3){x,y,z};
}

Light ce_new_directional_light(float power, Color color, float height, float angle)
{
    Vec3 dir = _light_direction_from_height_and_angle(height, angle);
    Light light = {
        .type= LIGHT_DIRECTIONAL,
        .pos=dir,
        .color=color,
        .power=power
    };
    return light;
}


AsciiPixel ce_new_ascii_pixel_three_bytes(Color bg, Color fg, const char *bytes) {
    AsciiPixel pixel = {.bg=bg, .fg=fg};
    memcpy(pixel.bytes, bytes, sizeof(char)*3);
    return pixel;
}

AsciiPixel ce_new_ascii_pixel_single_byte(Color bg, Color fg, const char *symbol) {
    AsciiPixel pixel = {.bg=bg, .fg=fg};
    memset(pixel.bytes, 0, 3*sizeof(char));
    memcpy(pixel.bytes, symbol, sizeof(char));
    return pixel;
}



size_t ce_import_texture(RenderSettings *renderSettings, char *filepath)
{
    TextureImage img = {0};
    bool sucess = _load_bmp_file(filepath, &img);
    
    if (!sucess) return -1;
    
    TextureImage *ptr = (TextureImage*)calloc(1, sizeof(TextureImage));
    if (ptr == NULL) return -1;

    *ptr = img;
    size_t index = renderSettings->_num_textures;
    renderSettings->textures[index] = ptr;
    printf("texture %s stored at index %zu\n", filepath, index);
    renderSettings->_num_textures++;

    return index;
}




static Normal get_interpolated_vertex_normal(Mesh *mesh, Face *face, Vec3 barycentrics){
    Normal na = mesh->normals[face->n0]; 
    Normal nb = mesh->normals[face->n1];
    Normal nc = mesh->normals[face->n2];
    Normal N = vec3_add(vec3_add(vec3_mul(na,barycentrics.z), vec3_mul(nb,barycentrics.x)), vec3_mul(nc,barycentrics.y));
    N = vec3_normalize(N);
    return N;
}

static Normal get_interpolated_uv(Mesh *mesh, Face *face, Vec3 barycentrics){
    if (face->t0 < 0 || face->t1 < 0 || face->t2 < 0) { return (Vec3){0.,0.,0.};}
    Vec3 na = mesh->uvs[face->t0]; 
    Vec3 nb = mesh->uvs[face->t1];
    Vec3 nc = mesh->uvs[face->t2];
    return vec3_add(vec3_add(vec3_mul(na,barycentrics.z), vec3_mul(nb,barycentrics.x)), vec3_mul(nc,barycentrics.y));
}



// moeller trumbore ray face intersection
// returns the barycentric weights
static bool face_calculate_intersection(Mesh *mesh, Face *face, Vec3 ray_origin, Vec3 ray_dir, HitResult *out_hit)
{
    Normal face_n = face->normal;
    if (vec3_dot(ray_dir, face_n) > -0.00001f) { return false; }

    Vertex a = mesh->vertices[face->v0]; 
    Vertex b = mesh->vertices[face->v1];
    Vertex c = mesh->vertices[face->v2];
    Vec3 v0v1 = vec3_sub(b,a); 
    Vec3 v0v2 = vec3_sub(c,a);

    Vec3 pvec =  vec3_cross(ray_dir,v0v2);
    float det = vec3_dot(v0v1, pvec);
    if (det < 0.0000001) return false;


    float inverseDet = 1.f / det;
    Vec3 tvec = vec3_sub(ray_origin, a);
    float u   = vec3_dot(tvec, pvec) * inverseDet;
    if (u < 0 || u > 1) return false;

    Vec3 qvec = vec3_cross(tvec,v0v1);
    float v   = vec3_dot(ray_dir, qvec) * inverseDet;
    if (v < 0 || u + v > 1) return false;

    float w = 1.f - u - v;
    float depth = vec3_dot(v0v2, qvec) * inverseDet;

    out_hit->depth = depth;
    out_hit->normal = face_n;
    out_hit->face = face;
    out_hit->barycentric_weights = (Vec3) {u,v,w};
    return depth > 0;
}


// used in the BVH sampling
static bool calculate_bounding_box_intersection(
    Vec3 min, Vec3 max, Vec3 orig, Vec3 inv_dir, float* t_out) 
{
    // X Axis
    float tx1 = (min.x - orig.x) * inv_dir.x;
    float tx2 = (max.x - orig.x) * inv_dir.x;
    float tmin = fminf(tx1, tx2);
    float tmax = fmaxf(tx1, tx2);

    // Y Axis
    float ty1 = (min.y - orig.y) * inv_dir.y;
    float ty2 = (max.y - orig.y) * inv_dir.y;
    tmin = fmaxf(tmin, fminf(ty1, ty2));
    tmax = fminf(tmax, fmaxf(ty1, ty2));

    // Z Axis
    float tz1 = (min.z - orig.z) * inv_dir.z;
    float tz2 = (max.z - orig.z) * inv_dir.z;
    tmin = fmaxf(tmin, fminf(tz1, tz2));
    tmax = fminf(tmax, fmaxf(tz1, tz2));

    if (tmax >= tmin && tmax > 0.0f) {
        *t_out = tmin; 
        return true;
    }
    return false;
}




bool intersect_mesh_bvh(Mesh* mesh, Vec3 ray_orig, Vec3 ray_dir, Vec3 inv_dir, HitResult* record) {
    int stack[64];
    int stack_ptr = 0;
    
    stack[stack_ptr++] = 0;

    bool hit = false;
    HitResult temp_hit = {0};
    float best_depth = FLT_MAX; 

    while (stack_ptr > 0) {
        int node_idx = stack[--stack_ptr];
        BVHNode* node = &mesh->bvh_nodes[node_idx];

        float t_box;
        if (!calculate_bounding_box_intersection(node->bounds.min, node->bounds.max, ray_orig, inv_dir, &t_box)) {
            continue; 
        }

        if (t_box > best_depth) {
            continue;
        }

        if (node->face_count > 0) {
            for (int i = 0; i < node->face_count; i++) {
                int face_idx = mesh->face_indices[node->left_first + i];
                
                if (face_calculate_intersection(mesh, &mesh->faces[face_idx], ray_orig, ray_dir, &temp_hit)) {
                    if (temp_hit.depth < best_depth && temp_hit.depth > 0.0f) {
                        best_depth = temp_hit.depth;
                        *record = temp_hit;
                        hit = true;
                    }
                }
            }
        } else {
            int left_child = node->left_first;
            int right_child = node->left_first + 1;

            stack[stack_ptr++] = right_child;
            stack[stack_ptr++] = left_child;
        }
    }
    return hit;
}




/*
// old version without the bvh
static bool mesh_calculate_intersection(Mesh *mesh, Vec3 ray_origin, Vec3 ray_dir, HitResult *result)
{
    // first check for bounding box hit
    // and abort on no hit
    float x;
    Vec3 inv_dir = { 1.0f / ray_dir.x, 1.0f / ray_dir.y, 1.0f / ray_dir.z };
    if (!calculate_bounding_box_intersection(mesh->bounds_min, mesh->bounds_max, ray_origin, inv_dir, &x)) {
        return false;
    }

    float bestDepth = FLT_MAX;
    bool hit = false;
    HitResult temp_hit = {0};


    for (size_t i=0; i<mesh->face_count; i++) 
    {
        bool _hit = face_calculate_intersection(mesh, &mesh->faces[i], ray_origin, ray_dir, &temp_hit);
        if (_hit && temp_hit.depth < bestDepth) {
            hit = true;
            bestDepth = temp_hit.depth;
            //temp_face = &mesh->faces[i];
            //result->normal = temp_N;
            //result->barycentric_weights = temp_barycentric_weights;
            *result = temp_hit;
        }
    }
    //result->depth = bestDepth;
    //result->face  = temp_face; 
    return hit;
}
*/



void ce_add_object_to_scene(RenderSettings *renderSettings, Object *object)
{
    size_t i = renderSettings->scene.numObjects;
    renderSettings->scene.objects[i] = object;
    printf("object added at index %d\n",i);
    renderSettings->scene.numObjects += 1;
}

void ce_add_light_to_scene(RenderSettings *renderSettings, Light *light)
{
    renderSettings->scene.lights[renderSettings->scene.numLights] = light;
    renderSettings->scene.numLights += 1;
}














static void set_cursor_visibility(bool visible) {
    HANDLE qConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO lpCursor;
    GetConsoleCursorInfo(qConsole, &lpCursor);
    lpCursor.bVisible = visible;
    SetConsoleCursorInfo(qConsole, &lpCursor);
}

bool get_terminal_size(uint8_t *x, uint8_t *y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        *x = csbi.srWindow.Right - csbi.srWindow.Left + 0;
        *y = csbi.srWindow.Bottom - csbi.srWindow.Top + 0;
        return true;
    }
    return false;
}




RenderSettings* ce_create_rendersettings()
{
    RenderSettings* renderer = calloc(1,sizeof(RenderSettings));
    if (!renderer) return NULL;
    Camera cam;
    uint8_t width, height;
    if (!get_terminal_size(&width,&height)){
        printf("could not get terminal size\nDefaulting to 100x100 for debugging");
        width = 100;
        height = 100;
        //return NULL;
    }

    camera_init(&cam, width, height);
    

    // ANSI processing in Windows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    set_cursor_visibility(false);
    SetConsoleOutputCP(CP_UTF8);
    

    // Set properties
    renderer->render_height = height;
    renderer->render_width  = width;
    renderer->camera = cam;
    renderer->current_frame = 0;
    //renderer->color_mode = REGULAR;
    renderer->background_color = color_new_mono(0.f);
    renderer->environment = NULL;
    
    renderer->_num_textures = 0;

    renderer->fog_enabled = false;
    renderer->fog_start = 3.0f;
    renderer->fog_end = 30.0f;
    
    renderer->_grey_box_material = calloc(1,sizeof(Material));
    *renderer->_grey_box_material = ce_new_material_full((Color){0.75f,0.75f,0.75f}, 1.0f);

    memset(&renderer->scene, 0, sizeof(renderer->scene));
    

    // Setup the Canvas Dimensions
    renderer->canvas.width = width;
    renderer->canvas.height = height;

    renderer->_temp_canvas.width = width;
    renderer->_temp_canvas.height = height;
    size_t total_pixels = (size_t)width * (size_t)height;


    // Allocate the buffers onto the heap
    renderer->canvas.pixels = malloc(total_pixels * sizeof(AsciiPixel));
    if (!renderer->canvas.pixels) {
        free(renderer);
        return NULL;
    }
    // Allocate the intermediate buffer for post processing
    renderer->_temp_canvas.pixels = malloc(total_pixels * sizeof(AsciiPixel));
    if (!renderer->_temp_canvas.pixels) {
        free(renderer);
        return NULL;
    }
    // allocate the depth buffer
    renderer->depth_buffer = calloc(total_pixels, sizeof(float));
    if (!renderer->depth_buffer) {
        free(renderer);
        return NULL;
    }
    // allocate the stencil buffer
    renderer->stencil_buffer = calloc(total_pixels, sizeof(uint8_t));
    if (!renderer->stencil_buffer) {
        free(renderer);
        return NULL;
    }

    ce_clear_canvas(renderer);
    
    return renderer;
}




// cast ray into scene and returns HitResult
static bool cast_ray(RenderSettings *renderSettings, Vec3 ray_origin, Vec3 ray_direction, HitResult *out_result)
{
    bool   hit = false;

    HitResult temp_result = {0};
    HitResult best_result = {.depth = FLT_MAX};

    for (size_t i=0; i<renderSettings->scene.numObjects; i++)
	{
        Object *current_object = renderSettings->scene.objects[i]; 
        Mesh *mesh = current_object->mesh;


        // transform camera by inverse object transform;
        Vec3 _direction = vec3_transform_direction(&current_object->inverse_transform, ray_direction);
        Vec3 _origin    = vec3_transform(&current_object->inverse_transform, ray_origin);
        
	    //bool _hit = mesh_calculate_intersection(mesh, _origin, _direction, &temp_result);

        Vec3 inv_dir = { 1.0f / _direction.x, 1.0f / _direction.y, 1.0f / _direction.z };
	    bool _hit = intersect_mesh_bvh(mesh, _origin, _direction, inv_dir, &temp_result);
        if (_hit && temp_result.depth<best_result.depth)
        {
            best_result        = temp_result;
            best_result.object = current_object;
            hit = true;
        }
    }

    if (!hit)
    {
        if (out_result != NULL) {
            *out_result = (HitResult) {0};
            out_result->did_hit = false;
            out_result->ray = (Ray){.direction=ray_direction, .origin=ray_origin};
        }
        return false;
        
    }

    // if we just check for shadows we don't supply a HitResult pointer
    if (out_result != NULL)
    {
        *out_result = best_result;
        out_result->did_hit = true;
        out_result->normal = vec3_transform_direction(&best_result.object->transform, best_result.normal);
        out_result->ray = (Ray){.direction = ray_direction, .origin=ray_origin};
    }
    return true;
}





bool calculate_shadow(RenderSettings *renderSettings, Vec3 position, Vec3 lightDir)
{
    Vec3 biased_origin = vec3_add( position, vec3_mul(lightDir, .001f));
	return cast_ray(renderSettings, biased_origin, lightDir, NULL);
}


bool cast_reflection_ray(RenderSettings *renderSettings, Vec3 position, Vec3 normal, Vec3 view_direction, HitResult *out_result)
{
    // negating view_direction so it points TOWARD the surface
    Vec3 incident = vec3_negate(view_direction);
    Vec3 reflect = vec3_reflect(incident, normal); 
    Vec3 biased_origin = vec3_add(position, vec3_mul(reflect, .001f));
    return cast_ray(renderSettings, biased_origin, reflect, out_result);
}



























// main material for fixed colors
// - will dither a gradient using only the input palette
// - use void ce_gradient_material_set_texture to assign texture
Material ce_new_material_gradient_ramp(Color *colors, uint8_t num_colors, float spec_amount, float dither_amount, const char **symbols, uint8_t num_symbols){
    ShaderParams_Surface_GradientRamp *params = calloc(1, sizeof(ShaderParams_Surface_GradientRamp));
    if (params == NULL) printf("could not allocate material parameters");

    params->brightness = 1.0f;
    params->colors = colors;
    params->num_colors = num_colors;
    
    params->symbols = symbols;
    params->num_symbols = num_symbols;
    params->ambient = 0.0f;

    params->spec_amount = spec_amount;
    params->spec_sharpness = 0.75f;
    params->dither_amount = dither_amount;
    params->discrete_steps=true;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_surface_gradient_ramp
    };
    return new;
}



// main material for smooth colors
// - will interpolate smoothly and apply lut if present
Material ce_new_material_gradient_ramp_interpolated( AsciiLUT *lut,Color *colors, uint8_t num_colors, float spec_amount, float dither_amount){
    Material new = ce_new_material_gradient_ramp(colors, num_colors, spec_amount, dither_amount, NULL, 0);
    ShaderParams_Surface_GradientRamp *sh = new.parameters;
    sh->discrete_steps=false;
    sh->lut=lut;
    return new;
}



// main material for fixed colors
// - will dither a gradient using only the target palette and only the simple block characters
// - use void ce_gradient_material_set_texture to assign texture
Material ce_new_material_gradient_ramp_blocks(Color *colors, uint8_t num_colors, float spec_amount, float dither_amount) {
    const char **symbols = CHARACTERS_BLOCK;
    uint8_t num_symbols = sizeof(CHARACTERS_BLOCK) / sizeof(CHARACTERS_BLOCK[0]);
    Material new = ce_new_material_gradient_ramp(colors, num_colors, spec_amount, dither_amount, symbols, num_symbols);
    ShaderParams_Surface_GradientRamp *sh = new.parameters;
    return new;
}



void ce_gradient_material_set_brightness(Material *mtl, float brightness){
    ShaderParams_Surface_GradientRamp *params = mtl->parameters;
    params->brightness = brightness;
}
void ce_gradient_material_set_ambient(Material *mtl, float ambient){
    ShaderParams_Surface_GradientRamp *params = mtl->parameters;
    params->ambient = ambient;
}

void ce_gradient_material_set_specular_sharpness(Material *mtl, float sharpness){
    ShaderParams_Surface_GradientRamp *params = mtl->parameters;
    params->spec_sharpness = sharpness;
}

void ce_gradient_material_set_texture(Material *mtl, uint8_t tex_id){
    ShaderParams_Surface_GradientRamp *sh = mtl->parameters;
    sh->use_texture=true;
    sh->tex_id=tex_id;
}

void ce_gradient_material_set_dither_pattern(Material *mtl, DitherPattern dither_pattern){
    ShaderParams_Surface_GradientRamp *sh = mtl->parameters;
    sh->dither_pattern = dither_pattern;
}





// use tile prefabs to scatter based on luminance
Material ce_new_material_tiles(AsciiPixel *tiles, size_t num_tiles, float spec_amount, float dither_amount){
    ShaderParams_Surface_Tiles *params = calloc(1, sizeof(ShaderParams_Surface_Tiles));
    if (params == NULL) printf("could not allocate material parameters");
    params->brightness = 1.0f;
    params->tiles = tiles;
    params->num_tiles = num_tiles;
    params->ambient = 0.0f;
    params->spec_amount = spec_amount;
    params->spec_sharpness = 0.75f;
    params->dither_amount = dither_amount;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_surface_tiles
    };
    return new;
}

void ce_tile_material_set_brightness(Material *mtl, float brightness){
    ShaderParams_Surface_Tiles *sh = mtl->parameters;
    sh->brightness = brightness;
}

void ce_tile_material_set_texture(Material *mtl, uint8_t tex_id){
    ShaderParams_Surface_Tiles *sh = mtl->parameters;
    sh->use_texture=true;
    sh->tex_id=tex_id;
}



























Material ce_new_material_chrome(AsciiLUT *lut, Color tint, float dither_amount, uint8_t tex_id){
    ShaderParams_Surface_Chrome *params = calloc(1, sizeof(ShaderParams_Surface_Chrome));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount=dither_amount;
    params->tex_id=tex_id;
    params->tint=tint;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_surface_chrome
    };
    return new;
}



Material ce_new_material_environment_texture(AsciiLUT *lut, EnvironmentProjectionMethod method, float brightness, float dither_amount, uint8_t tex_id){
    ShaderParams_Environment_Texture *params = calloc(1, sizeof(ShaderParams_Environment_Texture));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->method = method;
    params->brightness=brightness;
    params->dither_amount=dither_amount;
    params->tex_id=tex_id;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_environment_texture
    };
    return new;
}






Material ce_new_material_principled(AsciiLUT *lut, Color color, float spec_amount, float dither_amount){
    ShaderParams_Surface_Principled *params = calloc(1, sizeof(ShaderParams_Surface_Principled));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->color = color;
    params->spec_amount = spec_amount;
    params->spec_sharpness = 0.75f;
    params->dither_amount = dither_amount;
    params->fog_enabled = true;
    params->rim_light_amount = 0.125f;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_surface_principled
    };
    return new;
}

// debug grey material with no dithering, lut or fog
Material ce_new_material_full(Color color, float spec_amount){
    Material new = ce_new_material_principled(NULL, color, spec_amount, 0.0f);
    ShaderParams_Surface_Principled *sh = new.parameters;
    sh->fog_enabled=false;
    return new;
}

void ce_principled_material_set_texture(Material *mtl, uint8_t tex_id){
    ShaderParams_Surface_Principled *sh = mtl->parameters;
    sh->use_texture=true;
    sh->tex_id=tex_id;
}
void ce_principled_material_set_ambient(Material *mtl, float amount){
    ShaderParams_Surface_Principled *sh = mtl->parameters;
    sh->ambient = amount;
}
// fresnel 0-1 where 1 is chrome
// also set the rim light effect to 0
void ce_principled_material_set_reflection(Material *mtl, float mult, float fresnel){
    ShaderParams_Surface_Principled *sh = mtl->parameters;
    sh->reflection_amount = mult;
    sh->reflection_fresnel = fresnel;
    sh->rim_light_amount = 0;
}

void ce_principled_material_set_rim_light(Material *mtl, float amount){
    ShaderParams_Surface_Principled *sh = mtl->parameters;
    sh->rim_light_amount = amount;
}







Material ce_new_material_post_lut(AsciiLUT *lut, float dither_amount)
{
    ShaderParams_Post_LUT *params = calloc(1, sizeof(ShaderParams_Post_LUT));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount = dither_amount;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_post_lut
    };
    return new;
}

\

Material ce_new_material_post_bloom(AsciiLUT *lut, float dither_amount)
{
    ShaderParams_Post_Bloom *params = calloc(1, sizeof(ShaderParams_Post_Bloom));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount = dither_amount;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_post_bloom
    };
    return new;
}

Material ce_new_material_post_blur(AsciiLUT *lut, size_t x_samples, size_t y_samples, float dither_amount)
{
    ShaderParams_Post_Blur *params = calloc(1, sizeof(ShaderParams_Post_Blur));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount = dither_amount;
    params->x_samples = x_samples;
    params->y_samples = y_samples;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_post_blur
    };
    return new;
}


Material ce_new_material_post_dispersion(AsciiLUT *lut, size_t num_samples, size_t size, float dither_amount)
{
    ShaderParams_Post_Dispersion *params = calloc(1, sizeof(ShaderParams_Post_Dispersion));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount = dither_amount;
    params->num_samples = num_samples;
    params->size = size;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_post_dispersion
    };
    return new;
}




Material ce_new_material_post_horizontal_glitches(AsciiLUT *lut, size_t num_samples, size_t stride, float dropout_chance,float dither_amount)
{
    ShaderParams_Post_Horizontal_Glitches *params = calloc(1, sizeof(ShaderParams_Post_Horizontal_Glitches));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount = dither_amount;
    params->num_samples = num_samples;
    params->stride = stride;
    params->dropout_chance = dropout_chance;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_post_horizontal_glitches
    };
    return new;
}

Material ce_new_material_post_horizontal_blur(AsciiLUT *lut, size_t num_samples, size_t stride, float dropout_chance,float dither_amount)
{
    ShaderParams_Post_Horizontal_Glitches *params = calloc(1, sizeof(ShaderParams_Post_Horizontal_Glitches));
    if (params == NULL) printf("could not allocate material parameters");
    params->lut = lut;
    params->dither_amount = dither_amount;
    params->num_samples = num_samples;
    params->stride = stride;
    params->dropout_chance = dropout_chance;
    Material new = {
        .parameters = (void*)params,
        .shader = shader_post_horizontal_blur
    };
    return new;
}




// converts a ray (primary or secondary) into an actual fragment color
// can be called from primary render_pixel function or for secondary rays
bool process_ray(RenderSettings *renderSettings, size_t x_coord, size_t y_coord, HitResult *hit_result, AsciiPixel *out_pixel)
{
    // No geometry hit
    if (!hit_result->did_hit) 
    {
        if (renderSettings->render_mode == RENDERMODE_GREYBOX) return false;
        if (renderSettings->environment == NULL) return false;
        // render environment
        FragmentData frag = {
            .camera_position = hit_result->ray.origin,
            .view_direction =  hit_result->ray.direction,
            .x_coord = x_coord,
            .y_coord = y_coord
        };
        Material *env_material = renderSettings->environment;
        env_material->shader(renderSettings, &frag, env_material->parameters, out_pixel);
        return true;
    }


    // We have a geometry hit and calculate our stuff
    const float depth    = hit_result->depth;
    const Vec3 pos        = vec3_add(hit_result->ray.origin, vec3_mul(hit_result->ray.direction, depth));
    const Vec3 viewDir    = vec3_normalize( vec3_sub( hit_result->ray.origin, pos));

    // calculate the final hit normal by interpolating the vertex normals
    Normal normal       = get_interpolated_vertex_normal(hit_result->object->mesh,hit_result->face, hit_result->barycentric_weights);
    normal              = vec3_transform_direction(&hit_result->object->transform, normal);

    // calculate the final hit uv by interpolating the vertex uvs
    Vec3 uv             = get_interpolated_uv(hit_result->object->mesh,hit_result->face, hit_result->barycentric_weights);
    Face *face          = hit_result->face;

   
    Material *material   = hit_result->object->materials[face->material_id]; 
    if (renderSettings->render_mode == RENDERMODE_GREYBOX) material = renderSettings->_grey_box_material;
    
    
    // assing ERROR magenta if no material assigned 
    if (material == NULL) {
        out_pixel->bg = C_ANSI_MAGENTA;
        out_pixel->fg = C_ANSI_MAGENTA;
        return false;
    }

    FragmentData frag = {
        .camera_position = renderSettings->camera.pos,
        .view_direction = viewDir,
        .normal = normal,
        .pos = pos,
        .depth = depth,
        .x_coord = x_coord,
        .y_coord = y_coord,
        .uv = uv
    };

    material->shader(renderSettings, &frag, material->parameters, out_pixel);
    renderSettings->depth_buffer[x_coord + renderSettings->render_width*y_coord] = depth;
    renderSettings->stencil_buffer[x_coord + renderSettings->render_width*y_coord] = material->stencil;
    return true;
}



// casts the eye ray
// should only be called from the main render function
// calls ce_process_ray on the result for further processing
// returns true if either geometry was hit or we rendered an environment (not used atm afik)
static bool render_pixel(RenderSettings *renderSettings, uint8_t x, uint8_t y, AsciiPixel *out_pixel)
{
    
    float u = (((float)x / renderSettings->render_width) - 0.5f) / 2.0f; // divide by 2 to account for non square character ratio
    float v = 0.5f - ((float)y / renderSettings->render_height);
    Camera cam = renderSettings->camera;

    const Ray ray = camera_ray(&cam, u, v);


    HitResult hit_result = {0};
    cast_ray(renderSettings, ray.origin, ray.direction, &hit_result);
    
    return process_ray(renderSettings, x, y, &hit_result, out_pixel);
    

    // No geometry hit
    if (!hit_result.did_hit) 
    {
        if (renderSettings->render_mode == RENDERMODE_GREYBOX) return false;
        // there is no valid environment texture assigned
        if (renderSettings->environment == NULL) return false;
        
        // render environment
        FragmentData frag = {
            .camera_position = cam.pos,
            .view_direction = ray.direction,
            .x_coord = x,
            .y_coord = y
        };
        Material *env_material = renderSettings->environment;
        env_material->shader(renderSettings, &frag, env_material->parameters, out_pixel);
        return true;
    }


    // We have a hit and calculate our stuff
    const float depth    = hit_result.depth;
    const Vec3 pos        = vec3_add(ray.origin, vec3_mul(ray.direction, depth));
    const Vec3 viewDir    = vec3_normalize( vec3_sub( ray.origin, pos));

    // calculate the final hit normal by interpolating the vertex normals
    //Normal normal        = hit_result.normal;
    Normal normal       = get_interpolated_vertex_normal(hit_result.object->mesh,hit_result.face, hit_result.barycentric_weights);
    Vec3 uv             = get_interpolated_uv(hit_result.object->mesh,hit_result.face, hit_result.barycentric_weights);
    normal              = vec3_transform_direction(&hit_result.object->transform, normal);
    Face *face          = hit_result.face;

   
    Material *material   = hit_result.object->materials[face->material_id]; 
    if (renderSettings->render_mode == RENDERMODE_GREYBOX) material = renderSettings->_grey_box_material;
    
    
    // No material assigned
    if (material == NULL) {
        out_pixel->bg = C_PICO8_PINK;
        out_pixel->fg = C_PICO8_PINK;
        return false;
    }

    FragmentData frag = {
        .camera_position = cam.pos,
        .view_direction = viewDir,
        .normal = normal,
        .pos = pos,
        .depth = depth,
        .x_coord = x,
        .y_coord = y,
        .uv = uv
    };

    material->shader(renderSettings, &frag, material->parameters, out_pixel);
    renderSettings->depth_buffer[x + renderSettings->render_width*y] = depth;
    renderSettings->stencil_buffer[x + renderSettings->render_width*y] = material->stencil;
    return true;
}





void ce_render_frame(RenderSettings *renderSettings)
{
    if (renderSettings==NULL) return;
    AsciiCanvas *canvas = &renderSettings->canvas;
    
    //render_scene(renderSettings);
    //return;

    #pragma omp parallel for collapse(2) schedule(dynamic, 16)
    for (size_t y = 0; y<canvas->height; y++)
    {
        for (size_t x = 0; x<canvas->width; x++)
        {
            Color color;
            float luma;
            AsciiPixel current_pixel;
            size_t xy = x + y*canvas->width;
            bool hit = render_pixel( renderSettings, x, y, &canvas->pixels[xy]);
        }
    }
    
}


void cycle_modes(RenderSettings *settings)
{
    size_t m = settings->render_mode + 1;
    m = fmodf(m, NUM_RENDER_MODES);
    settings->render_mode = (RenderMode)m;
}



// A quick inline helper to stream numbers straight into bytes
static inline char* append_int_to_buffer(char *ptr, uint8_t val) {
    if (val >= 100) {
        *ptr++ = '0' + (val / 100);
        *ptr++ = '0' + ((val / 10) % 10);
        *ptr++ = '0' + (val % 10);
    } else if (val >= 10) {
        *ptr++ = '0' + (val / 10);
        *ptr++ = '0' + (val % 10);
    } else {
        *ptr++ = '0' + val;
    }
    return ptr;
}

void ce_display_canvas(RenderSettings *renderSettings) {
    AsciiCanvas *canvas = &renderSettings->canvas;
    static char output_buffer[2048 * 1536 * 3];
    char *ptr = output_buffer;

    printf("\x1b[H"); // reset cursor position to top-left


    for (int r = 0; r < canvas->height; r++) {
        for (int c = 0; c < canvas->width; c++) 
        {
            size_t index = c + r*canvas->width;
            Color col = canvas->pixels[index].fg;
            Color bg = canvas->pixels[index].bg;
            
            // foreground full color -> prefix: "\x1b[38;2;"
            *ptr++ = '\x1b'; *ptr++ = '['; 
            *ptr++ = '3'; *ptr++ = '8'; *ptr++ = ';'; 
            *ptr++ = '2'; *ptr++ = ';';
            // write the channels
            ptr = append_int_to_buffer(ptr, col.x*255); *ptr++ = ';';
            ptr = append_int_to_buffer(ptr, col.y*255); *ptr++ = ';';
            ptr = append_int_to_buffer(ptr, col.z*255); *ptr++ = 'm';
            

            // background color (\x1b[48;2;R;G;Bm)
            *ptr++ = '\x1b'; *ptr++ = '['; 
            *ptr++ = '4'; *ptr++ = '8'; *ptr++ = ';';
            *ptr++ = '2'; *ptr++ = ';';
            // write the channels
            ptr = append_int_to_buffer(ptr, bg.x*255); *ptr++ = ';';
            ptr = append_int_to_buffer(ptr, bg.y*255); *ptr++ = ';';
            ptr = append_int_to_buffer(ptr, bg.z*255); *ptr++ = 'm';

            // write our symbol (max 3 bytes)
            const char* sym = canvas->pixels[index].bytes;
            if (sym[0] != '\0') *ptr++ = sym[0];
            if (sym[1] != '\0') *ptr++ = sym[1];
            if (sym[2] != '\0') *ptr++ = sym[2];
        }
        // end-of-line reset parameters
        *ptr++ = '\x1b'; *ptr++ = '['; *ptr++ = '0'; *ptr++ = 'm';
        *ptr++ = '\x1b'; *ptr++ = '['; *ptr++ = 'K';
        *ptr++ = '\n';
    }
    size_t total_bytes = ptr - output_buffer;
    fwrite(output_buffer, 1, total_bytes, stdout);
    fflush(stdout);
}



/*
void free_renderer(RenderSettings *renderSettings){
    for (size_t i = 0; i < renderSettings->scene.numMeshes; i++) {
        Mesh **mesh_slot = &renderSettings->scene.meshes[i];
        if (*mesh_slot != NULL) {   // de-reference it once (*) to check if a mesh exists in that slot
            free_mesh(*mesh_slot);  // clean up mesh
            *mesh_slot = NULL;      // set the slot to NULL
        }
    }
}
*/


int get_key_async() {
    if (_kbhit()) return _getch();
    return -1;
}


int key_down(int key)
{
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

// use the Z and X Buttons to rotate the object around Y
void ce_object_manual_turntable(Object *obj)
{
    if (key_down('Z')) ce_rotate_y(obj,  0.1f);
    if (key_down('X')) ce_rotate_y(obj, -0.1f);
}


void ce_clear_canvas(RenderSettings *settings){
    AsciiPixel px = {
        .bg = settings->background_color,
        .fg = settings->background_color
    };
    px.bytes[0] = ' ';
    px.bytes[1] = '\0';
    px.bytes[2] = '\0';
    for (int r = 0; r < settings->canvas.height; r++) {
        for (int c = 0; c < settings->canvas.width; c++) 
        {
            size_t index = c + r*settings->canvas.width;
            settings->canvas.pixels[index]  = px;
            settings->depth_buffer[index]   = FLT_MAX;
            settings->stencil_buffer[index] = 0;
        }
    }
}


bool ce_play(RenderSettings *settings)
{
    bool running = true;
    settings->current_frame++;

    //printf("\x1b[2J"); // Clear screen completely once at start

    if (key_down(27)) {
        return false;
    };

    if (key_down('W')) camera_walk(&settings->camera, .1f);
    if (key_down('S')) camera_walk(&settings->camera, -.1f);
    if (key_down('D')) camera_strafe(&settings->camera, .01f);
    if (key_down('A')) camera_strafe(&settings->camera, -.01f);
    if (key_down('Q')) camera_zoom(&settings->camera, 1.0f);
    if (key_down('E')) camera_zoom(&settings->camera, -1.0f);

    // targeted cam
    // ////////////////
    if (settings->camera.is_targeted){
        if (key_down('D')) camera_strafe(&settings->camera,  0.25f);
        if (key_down('A')) camera_strafe(&settings->camera, -0.25f);
        if (key_down('I')) camera_strafe_vertical(&settings->camera, .1f);
        if (key_down('K')) camera_strafe_vertical(&settings->camera, -.1f);
    }
    // free cam
    // ////////////////
    else{
        if (key_down('D')) camera_strafe(&settings->camera, .05f);
        if (key_down('A')) camera_strafe(&settings->camera, -.05f);
        if (key_down('J')) camera_rotate_yaw(&settings->camera,  .025f);
        if (key_down('L')) camera_rotate_yaw(&settings->camera, -.025f);
        if (key_down('I')) camera_rotate_pitch(&settings->camera, .025f);
        if (key_down('K')) camera_rotate_pitch(&settings->camera,  -.025f);
        if (key_down('U')) camera_strafe_vertical(&settings->camera, .1f);
        if (key_down('O')) camera_strafe_vertical(&settings->camera, -.1f);
    }

    // use get_key_async for single presses
    // feels nicer
    int key = get_key_async();
    switch (key) {
        case 'm': cycle_modes(settings); break;
        case 'p': camera_toggle_orthographic_projection(&settings->camera); break;
        case 27: return false;
    }

    return running;
}




