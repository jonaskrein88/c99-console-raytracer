#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "vector.h"
#include "renderer.h"
#include "graphics2D.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


void ce_write_text(RenderSettings *settings, uint8_t x, uint8_t y, Color bg, Color fg, const char *txt)
{
    //char *txt = "This is a simple text\nthat I want to write to the screen!!!";

    //uint8_t x = 25;
    //uint8_t y = 50;

    uint8_t current_col = x;
    uint8_t current_row = y;

    for (size_t i = 0; txt[i] != '\0'; i++) 
    {
        if (txt[i] == '\n') {
            current_row++;
            current_col=x;
            continue;
        }
        size_t index = current_col + current_row * settings->canvas.width;
        settings->canvas.pixels[index].fg = fg;
        settings->canvas.pixels[index].bg = bg;
        settings->canvas.pixels[index].bytes[0] = txt[i];
        settings->canvas.pixels[index].bytes[1] = '\0';
        settings->canvas.pixels[index].bytes[2] = '\0';
        current_col++;
    }
}





void ce_draw_post_processing(RenderSettings *renderSettings, bool use_write_back, Material *mtl)
{
    //if (renderSettings==NULL) return;
    AsciiCanvas *target_canvas;
    AsciiCanvas *canvas = &renderSettings->canvas;


    
    // if we need a write back to avoid race conditions we set the scratch buffer as target
    // otherwise we write directly to the canvas to avoid the overhead
    if (use_write_back){
        target_canvas = &renderSettings->_temp_canvas;
        memcpy(target_canvas->pixels, canvas->pixels, canvas->width*canvas->height*sizeof(AsciiPixel));
    }
    else {
        target_canvas = canvas;
    }

    

    #pragma omp parallel for collapse(2) schedule(dynamic, 32)
    for (size_t y = 0; y<canvas->height; y++)
    {
        for (size_t x = 0; x<canvas->width; x++)
        {
            Vec3 uv = {
                ((float)x + 0.5f) / (float)canvas->width,
                1.0f - (((float)y + 0.5f) / (float)canvas->height),
                0.0f
            };
            
            //Vec3 camera_dir = camera_ray(&renderSettings->camera, uv.x, uv.y);
            size_t xy = x + y*canvas->width;
            FragmentData frag = {
                .x_coord = x,
                .y_coord = y,
                .uv = uv,
                //.view_direction = camera_dir,
                .camera_position = renderSettings->camera.pos
            };

            mtl->shader(renderSettings, &frag, mtl->parameters, &target_canvas->pixels[xy]);
            //renderSettings->canvas.pixels[xy].bg = C_GREEN;
        }
    }
   
    

    // write back to canvas
    if (use_write_back) {
        #pragma omp parallel for collapse(2) schedule(dynamic, 32)
        for (size_t y = 0; y<canvas->height; y++)
        {
            for (size_t x = 0; x<canvas->width; x++)
            {
                size_t xy = x + y*canvas->width;
                memcpy(&canvas->pixels[xy], &target_canvas->pixels[xy], sizeof(AsciiPixel));
            }
        }
    }
}



void ce_draw_convex_polygon(RenderSettings *renderSettings, Vec2 *vertices, int vertex_count, Color color) {
    if (vertex_count < 3) return;


    int min_y = vertices[0].y;
    int max_y = vertices[0].y;
    
    for (int i = 1; i < vertex_count; i++) {
        if (vertices[i].y < min_y) min_y = vertices[i].y;
        if (vertices[i].y > max_y) max_y = vertices[i].y;
    }
    // 1. SAFE CLIPPING: Guard vertical bounds to prevent out-of-bounds loops
    min_y = MAX(0, min_y);
    max_y = MIN(renderSettings->render_height - 1, max_y);

    for (int y = min_y; y <= max_y; y++) {
        int left_x = renderSettings->render_width;
        int right_x = -1;

        for (int i = 0; i < vertex_count; i++) {
            Vec2 p1 = vertices[i];
            Vec2 p2 = vertices[(i + 1) % vertex_count];

            if (p1.y > p2.y) { Vec2 temp = p1; p1 = p2; p2 = temp; }

            // 2. SAFE INTERSECTION: Only compute if the scanline actually cuts the edge
            if (y >= p1.y && y < p2.y) {
                int16_t dy = (int16_t)p2.y - (int16_t)p1.y;
                if (dy > 0) {
                    // Use a 64-bit int temporarily to completely avoid integer multiplication overflow
                    int64_t intersect_x = p1.x + ((int64_t)(y - p1.y) * (p2.x - p1.x)) / dy;
                    
                    left_x = MIN(left_x, (int)intersect_x);
                    right_x = MAX(right_x, (int)intersect_x);
                }
            }
        }

        // 3. SAFE HORIZONTAL CLIPPING: Clamp to pixel grid edges
        left_x = MAX(0, left_x);
        right_x = MIN(renderSettings->render_width - 1, right_x);

        if (left_x <= right_x) {
            AsciiPixel *pixel_row = &renderSettings->canvas.pixels[y * renderSettings->render_width + left_x];
            int count = right_x - left_x + 1;
            
            // Loop unrolling for maximum CPU cache filling
            while (count >= 4) {
                pixel_row[0].bg = color;
                pixel_row[1].bg = color;
                pixel_row[2].bg = color;
                pixel_row[3].bg = color;
                pixel_row += 4; count -= 4;
            }
            while (count > 0) {
                (pixel_row++)->bg = color;
                count--;
            }
        }
    }
}




void ce_draw_rectangle(RenderSettings *renderSettings, uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color) {
        
    for (size_t i = 0; i<w; i++){
        for (size_t j = 0; j<h; j++){
            uint16_t idx = x+i + renderSettings->render_width*(y+j); 
            memcpy( renderSettings->canvas.pixels[idx].bytes, " \0\0", 3*sizeof(char));
            renderSettings->canvas.pixels[idx].fg = color;
            renderSettings->canvas.pixels[idx].bg = color;
        }
    }
        return ;
    // 1. Calculate bounding boxes
    int x1 = x;
    int y1 = y;
    int x2 = x + w - 1;
    int y2 = y + h - 1;
    uint16_t width = renderSettings->render_width;
    uint16_t height = renderSettings->render_height;

    // 2. Direct Clipping: Instantly drop the shape if completely out of frame
    if (x2 < 0 || x1 >= width || y2 < 0 || y1 >= height) return;

    // Clamp coordinates to screen boundaries
    x1 = MAX(0, x1);
    y1 = MAX(0, y1);
    x2 = MIN(width - 1, x2);
    y2 = MIN(width - 1, y2);

    int row_pixels = x2 - x1 + 1;

    // 3. Blazing Fast Contiguous Memory Writing
    for (int current_y = y1; current_y <= y2; current_y++) {
        AsciiPixel *pixel_row = &renderSettings->canvas.pixels[current_y * width + x1];
        
        // If your color is a single repeating byte pattern (like 0xFFFFFFFF for white or 0x00000000 for black), 
        // you can use the standard C library's ultra-optimized memset:
        // memset(pixel_row, color, row_pixels * sizeof(uint32_t));
        
        // Otherwise, use basic 32-bit loop unrolling:
        int count = row_pixels;
        while (count >= 4) {
            pixel_row[0].bg = color; 
            pixel_row[1].bg = color;
            pixel_row[2].bg = color; 
            pixel_row[3].bg = color;
            pixel_row[0].fg = color; 
            pixel_row[1].fg = color;
            pixel_row[2].fg = color; 
            pixel_row[3].fg = color;
            pixel_row += 4; count -= 4;
        }
        while (count > 0) {
            (*pixel_row++).bg = color;
            (*pixel_row++).fg = color;
            count--;
        }
    }
}


void ce_draw_sprite(RenderSettings *renderSettings, size_t pos_x, size_t pos_y, size_t width, size_t height, Material *mtl)
{
    //TextureImage *tex = renderSettings->textures[tex_id];
    //size_t img_width = tex->width;
    //size_t img_height = tex->height;
    //size_t anchor_pos = pos_x + pos_y* renderSettings->canvas.width;


    for (size_t x = 0; x<width; x++){
        for (size_t y = 0; y<height; y++){
            //size_t texel_id = x + y*width;
            Vec3 uv = {
                ((float)x + 0.5f) / (float)width,
                1.0f - (((float)y + 0.5f) / (float)height),
                0.0f
            };
            size_t target_x = pos_x + x;
            size_t target_y = pos_y + y;

            if (target_x >= renderSettings->canvas.width || target_y >= renderSettings->canvas.height) continue;

            size_t target_pixel = target_x + target_y * renderSettings->canvas.width;

            FragmentData frag = {.x_coord = target_x, .y_coord = target_y, .uv=uv};
            mtl->shader(renderSettings, &frag, mtl->parameters, &renderSettings->canvas.pixels[target_pixel]);
        }
    }
}

// main function to create a text window or just the outlines of one
// used by the info window functions
// outputs a vec2 with the top left coordinate as reference for text
Vec2 ce_draw_info_window(RenderSettings *renderSettings, uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color bg, Color fg, const char* title, const char* sub_title, const char* text, bool fill){

    uint16_t screen_width  = renderSettings->canvas.width;
    uint16_t screen_height = renderSettings->canvas.height;
    
    uint16_t offset = y * screen_width + x; 
    
    // clear section of the screen
    if (fill == true){
        ce_draw_rectangle(renderSettings, x,y,width,height, bg);
    }
    

    for (size_t i = 1; i<width-1; i++)
    {
        uint16_t idx = i;
        idx += offset;
        memcpy( renderSettings->canvas.pixels[idx].bytes, "═", 3*sizeof(char));
        renderSettings->canvas.pixels[idx].fg = fg;
        renderSettings->canvas.pixels[idx].bg = bg;

        idx = i + (height-1) * screen_width;
        idx += offset;
        memcpy( renderSettings->canvas.pixels[idx].bytes, "═", 3*sizeof(char));
        renderSettings->canvas.pixels[idx].fg = fg;
        renderSettings->canvas.pixels[idx].bg = bg;
    }

    for (size_t j = 1; j<height-1; j++)
    {
        uint16_t idx = j*screen_width;
        idx += offset;
        memcpy( renderSettings->canvas.pixels[idx].bytes, "║", 3*sizeof(char));
        renderSettings->canvas.pixels[idx].fg = fg;
        renderSettings->canvas.pixels[idx].bg = bg;

        idx += width-1;
        memcpy( renderSettings->canvas.pixels[idx].bytes, "║", 3*sizeof(char));
        renderSettings->canvas.pixels[idx].fg = fg;
        renderSettings->canvas.pixels[idx].bg = bg;
    }
    uint16_t idx = offset;
    memcpy( renderSettings->canvas.pixels[idx].bytes, "╔", 3*sizeof(char));
    renderSettings->canvas.pixels[idx].fg = fg;
    renderSettings->canvas.pixels[idx].bg = bg;

    idx = offset + width-1;
    memcpy( renderSettings->canvas.pixels[idx].bytes, "╗", 3*sizeof(char));
    renderSettings->canvas.pixels[idx].fg = fg;
    renderSettings->canvas.pixels[idx].bg = bg;

    idx = offset + (height-1) * screen_width + width-1;
    memcpy( renderSettings->canvas.pixels[idx].bytes, "╝", 3*sizeof(char));
    renderSettings->canvas.pixels[idx].fg = fg;
    renderSettings->canvas.pixels[idx].bg = bg;

    idx = offset + screen_width * (height-1);
    memcpy( renderSettings->canvas.pixels[idx].bytes, "╚", 3*sizeof(char));
    renderSettings->canvas.pixels[idx].fg = fg;
    renderSettings->canvas.pixels[idx].bg = bg;


    size_t n = strlen(title);
    
    size_t center = width/2;
    size_t start = center-n/2;

    for (size_t i = 0; title[i] != '\0'; i++)
    {
        size_t idx = start + i + offset;
        memset( renderSettings->canvas.pixels[idx].bytes, 0, 3*sizeof(char));
        memcpy( renderSettings->canvas.pixels[idx].bytes, &title[i], sizeof(char));
        renderSettings->canvas.pixels[idx].fg = fg;
        renderSettings->canvas.pixels[idx].bg = bg;
    }

    if (sub_title)
    {
        size_t n = strlen(sub_title);
        
        size_t start = center-n/2 + (height-1) * screen_width;

        for (size_t i = 0; sub_title[i] != '\0'; i++)
        {
            size_t idx = start + i + offset;
            memset( renderSettings->canvas.pixels[idx].bytes, 0, 3*sizeof(char));
            memcpy( renderSettings->canvas.pixels[idx].bytes, &sub_title[i], sizeof(char));
            renderSettings->canvas.pixels[idx].fg = fg;
            renderSettings->canvas.pixels[idx].bg = bg;
        }
    }

    if (text != NULL){
        ce_write_text(renderSettings, x+2, y+2, bg, fg, text);
    }
    
    return (Vec2) {x,y};
}






// create a text window or just the outlines of one with a drop shadow
// calls ce_draw_info_window
Vec2 ce_draw_info_window_with_shadow(RenderSettings *renderSettings, uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color bg, Color fg, const char* title, const char* sub_title, const char* text, bool fill, Color shadow_color)
{
    ce_draw_info_window(renderSettings, x,y, width, height, bg, fg, title, sub_title, text, fill);
    ce_draw_rectangle(renderSettings, x+1, y+height, width, 1, shadow_color);
    ce_draw_rectangle(renderSettings, x+width, y+1, 1, height, shadow_color);
}















// looks dope
void replace_char(RenderSettings *renderSettings, const char* title){

    for (size_t x = 0; x<renderSettings->canvas.width; x++)
    {
        for (size_t y = 0; y<renderSettings->canvas.height; y++)
        {
            uint16_t idx = x + y*renderSettings->canvas.width;
            memcpy( renderSettings->canvas.pixels[idx].bytes, "═", 3*sizeof(char));
        }
    }
}


void ce_draw_camera_info(RenderSettings *renderSettings, Color bg, Color fg)
{
        Vec2 pos = {5,5}; 
       
        char txt[128];
        if (renderSettings->camera.is_orthographic){
            snprintf(txt,128,
                "Ortho Size: %f\n"
                "Resolution: %d x %d\n"
                "X:      %f\n"
                "Y:      %f\n"
                "Z:      %f\n", 
                renderSettings->camera.ortho_size,
                renderSettings->render_width,
                renderSettings->render_height,
                renderSettings->camera.pos.x,
                renderSettings->camera.pos.y,
                renderSettings->camera.pos.z);
        }
        else {
            snprintf(txt,128,
                "Focal Dist: %f\n"
                "Resolution: %d x %d\n"
                "X:      %f\n"
                "Y:      %f\n"
                "Z:      %f\n", 
                renderSettings->camera.focalDist,
                renderSettings->render_width,
                renderSettings->render_height,
                renderSettings->camera.pos.x,
                renderSettings->camera.pos.y,
                renderSettings->camera.pos.z);
        }
        ce_draw_info_window(renderSettings, pos.x, pos.y,32,11, bg, fg, " Camera ", "", txt, true);
        //if (renderSettings->camera.is_targeted){
            char tmp[64];
            snprintf(tmp,128,
            "Pitch:  %f\n"
            "Yaw:    %f", renderSettings->camera.pitch,  renderSettings->camera.yaw);
        ce_write_text(renderSettings, pos.x + 2, pos.y+7, bg, fg, tmp);
        //}
}

void ce_draw_object_info(RenderSettings *renderSettings, Object *object, Color bg, Color fg, const char* title)
{
        Vec2 pos = {5,20}; 
        Vec3 euler = matrix_get_euler_angles(&object->transform);
       
        char txt[128];
        snprintf(txt,128,"Yaw:   %f\nPitch: %f\nRoll:  %f", euler.x,euler.y, euler.z);
        ce_draw_info_window(renderSettings, pos.x, pos.y,32,8, bg, fg, title, "", txt, true);
}

void ce_draw_title(RenderSettings *renderSettings, Color bg, Color fg, const char* title, const char* sub_title){

    uint16_t width  = renderSettings->canvas.width;
    uint16_t height = renderSettings->canvas.height;

    Vec2 anchor = ce_draw_info_window(renderSettings,0 ,0,width,height,bg, fg,title, sub_title, NULL, false);
    return;
}

void ce_draw_title_with_drop_shadow(RenderSettings *renderSettings, Color bg, Color fg, const char* title, const char* sub_title, Color shadow_color, Color desktop_color){
    
        // upper bar
        ce_draw_rectangle(renderSettings,0,0,renderSettings->render_width,1, desktop_color);
        // lower bar
        ce_draw_rectangle(renderSettings,0,renderSettings->render_height-2,renderSettings->render_width,2, desktop_color);

        // left
        ce_draw_rectangle(renderSettings,0,0,2,renderSettings->render_height, desktop_color);
        // right
        ce_draw_rectangle(renderSettings,renderSettings->render_width-2,0,2,renderSettings->render_height, desktop_color);

        ce_draw_info_window_with_shadow(renderSettings, 2, 1, renderSettings->render_width-4,renderSettings->render_height-3, bg, fg, title, sub_title, "", false, shadow_color); 
}
