#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "vector.h"
#include "renderer.h"



void ce_write_text(RenderSettings *settings, uint8_t x, uint8_t y, Color bg, Color fg, const char *txt);
void ce_draw_post_processing(RenderSettings *renderSettings, bool use_write_back, Material *mtl);
void ce_draw_convex_polygon(RenderSettings *renderSettings, Vec2 *vertices, int vertex_count, Color color);
void ce_draw_rectangle(RenderSettings *renderSettings, uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color);

void ce_draw_sprite(RenderSettings *renderSettings, size_t pos_x, size_t pos_y, size_t width, size_t height, Material *mtl);

Vec2 ce_draw_info_window(RenderSettings *renderSettings, uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color bg, Color fg, const char* title, const char* sub_title, const char* text, bool fill);
Vec2 ce_draw_info_window_with_shadow(RenderSettings *renderSettings, uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color bg, Color fg, const char* title, const char* sub_title, const char* text, bool fill, Color shadow_color);
void ce_draw_object_info(RenderSettings *renderSettings, Object *object, Color bg, Color fg, const char* title);
void ce_draw_camera_info(RenderSettings *renderSettings, Color bg, Color fg);
void ce_draw_title(RenderSettings *renderSettings, Color bg, Color fg, const char* title, const char* sub_title);
void ce_draw_title_with_drop_shadow(RenderSettings *renderSettings, Color bg, Color fg, const char* title, const char* sub_title, Color shadow_color, Color desktop_color);